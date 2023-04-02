/*
 * Copyright (c) 2020-2023 Jose Hernandez
 *
 * This file is part of ZxRaspberry.
 *
 * ZxRaspberry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ZxRaspberry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZxRaspberry.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "zxemulatorscreen.h"
#include "zxkeyboard.h"
#include "common/Z80emu.h"
#include "common/gui/zxdialog.h"
#include "common/gui/zxlabel.h"
#include "common/gui/zxrect.h"
#include "circle/bcmframebuffer.h"
#include <QPainter>
#include <QtGlobal>
#include <QtDebug>


unsigned int scale = 3;
bool flash = false;
unsigned int frameCounter = 0;


ZxEmulatorScreen::ZxEmulatorScreen(Z80emu *z80emu, ZxDisplay *pZxDisplay, QWidget *parent) :
    QWidget(parent),
    m_z80emu(z80emu),
    m_pZxDisplay(pZxDisplay)
{
    antiAliased = false;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_pBcmFrameBuffer = new CBcmFrameBuffer(352, 296, 4);
    m_pZxDisplay->Initialize(m_z80emu->getRam() + 0x4000, m_pBcmFrameBuffer);
    m_zxKeyboard = new ZxKeyboard();

    // Set the focus on this widget so that we can get keyboard events
    setFocus();
}

ZxEmulatorScreen::~ZxEmulatorScreen() {

    delete m_zxKeyboard;
}


QSize ZxEmulatorScreen::minimumSizeHint() const {

    return {static_cast<int>(scale * 352) /* 48 + 256 + 48 */, static_cast<int>(scale * 296) /*(48 + 192 + 56)*/};
}


QSize ZxEmulatorScreen::sizeHint() const {

    return {static_cast<int>(scale * 352) /* 48 + 256 + 48 */, static_cast<int>(scale * 296) /*(48 + 192 + 56)*/};
}


void ZxEmulatorScreen::keyPressEvent(QKeyEvent *event) {

    if (event->key() == Qt::Key_F1) {
        showDialog = !showDialog;
        qDebug() << ((showDialog) ? "Showing About box" : "Hiding about box");
        repaint();
    }
    else {
        m_zxKeyboard->keyPressEvent(*m_z80emu, *event);
    }
}


void ZxEmulatorScreen::paintEvent(QPaintEvent * /* event */) {

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, antiAliased);
    painter.save();

    // The flash changes his state every 16 screen frames
    if (++frameCounter % 16 == 0) {
        flash = !flash;
    }

    m_pZxDisplay->update(flash);

    if (showDialog) {
        auto zxDialog = ZxDialog(ZxRect(2, 12, 40, 10), "About ZX Raspberry");

        /*
         * The default printable characters (32 (space) to 127 (copyright)) are stored at the end of the Spectrum's ROM at
         * memory address 15616 (0x3D00) to 16383 (0x3FFF) and are referenced by the system variable CHARS which can be
         * found at memory address 23606/7. Interestingly, the value in CHARS is actually 256 bytes lower than the first
         * byte of the space character so that referencing a printable ASCII character does not need to consider the first
         * 32 characters. As such, the CHARS value (by default) holds the address 15360 (0x3C00).
         *
         * The UDG characters (Gr-A to Gr-U) are stored at the end of the Spectrum's RAM at memory address 65368 (0xFF58)
         * to 65535 (0xFFFF). As such, POKEing this address range has immediate effect on the UDG characters. The USR
         * keyword (when followed by a single quoted character) provides a quick method to reference these addresses from
         * BASIC. As with the printable characters, the location of the UDG characters is stored in the system variable UDG.
         *
         * Reference: https://enacademic.com/dic.nsf/enwiki/513468
         */
        zxDialog.insert(new ZxLabel(ZxRect(1, 2, 1, 1), "ZX Raspberry version 0.0.1"));
        zxDialog.insert(new ZxLabel(ZxRect(1, 3, 1, 1), "Copyright \x7F 2020-2023 Jose Hernandez"));
        zxDialog.insert(new ZxLabel(ZxRect(1, 6, 1, 1), "Build date: " __DATE__ " " __TIME__));

        zxDialog.draw(reinterpret_cast<uint8_t *>(m_pBcmFrameBuffer->GetBuffer()));
    }

/* Reads Raspberry Pi framebuffer and turns it into a QImage object.
 *
 * Image data in the framebuffer is stored linearly starting with pixel (0,0) on the top left and the moving right
 * and down one pixel at a time.  In our particular example we have 2 pixels per byte and the bytes are out of
 * sequence because they are stored as a single 32-bit unsigned integer instead of 4 individual bytes.
 */
#if BYTE_SWAP_DISABLED
    for (int i = 0; i < 296; i++) {
        for (int j = 0; j < 44; j++) {
            for (int k=0; k<4; k++) {
                uint8_t byte = videoMemory.at( (i * 176) + (j * 4) + (3 - k) );
                image.setPixel( (j * 8) + (k * 2), i, m_pBcmFrameBuffer->palette[(byte & 0xF0u) >> 0x04u] );
                image.setPixel( (j * 8) + (k * 2) + 1 , i, m_pBcmFrameBuffer->palette[byte & 0x0Fu] );
            }
        }
    }
#else
    // Iterate over all rows. i.e. 0 to 296
    for (int i = 0x0000; i < 0x0128; i++) {
        // Iterate over all columns 2 pixels at a time. i.e. 0 to 176 (176 pixel pairs = 352 screen width / 2)
        for (int j = 0x0000; j < 0x00B0; j++) {
            uint8_t byte = reinterpret_cast<char *>(m_pBcmFrameBuffer->GetBuffer())[(i * 0x00B0) + j];
            image.setPixel((j * 2), i, m_pBcmFrameBuffer->palette[(byte & 0xF0u) >> 0x04u]);
            image.setPixel((j * 2) + 1, i, m_pBcmFrameBuffer->palette[byte & 0x0Fu]);
        }
    }
#endif

    // http://www.zxdesign.info/vidparam.shtml
    painter.drawImage(QRect(0, 0, static_cast<int>(scale * 352), static_cast<int>(scale * 296)), image);

    painter.restore();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
}
