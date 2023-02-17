#include <QPainter>
#include <QtGlobal>
#include <QtDebug>
#include <iostream>
#include "common/gui/zxdialog.h"
#include "common/gui/zxlabel.h"
#include "common/gui/zxrect.h"
#include "common/ViajeAlCentroDeLaTierraScr.h"
#include "screen.h"

int scale = 2;

Screen::Screen(QWidget *parent) : QWidget(parent)
{
    antiAliased = false;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    bcmFrameBuffer = new CBcmFrameBuffer(352, 272, 4);
    m_zxDisplay.Initialize(ViajeAlCentroDeLaTierra_scr, bcmFrameBuffer);
}

QSize Screen::minimumSizeHint() const
{
    return {scale * 352 /* 48 + 256 + 48 */, scale * 272 /*(32 + 192 + 48)*/};
}

QSize Screen::sizeHint() const
{
    return {scale * 352 /* 48 + 256 + 48 */, scale * 272 /*(32 + 192 + 48)*/};
}

void Screen::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1)
    {
        showDialog = !showDialog;
        repaint();
    }
}

void Screen::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, antiAliased);
    painter.save();
    m_zxDisplay.update(false);

    QByteArray videoMemory = QByteArray::fromRawData((const char *) bcmFrameBuffer->GetBuffer(), bcmFrameBuffer->GetSize());

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

        zxDialog.draw(reinterpret_cast<uint8_t *>(bcmFrameBuffer->GetBuffer()));
    }

#if 0
    zxDialog->printText(reinterpret_cast<uint8_t *>(bcmFrameBuffer->GetBuffer()), 1, 30, 0xD, 0x0, "SCREEN DIMENSIONS:");
    char const *innerDimensions = "Inner: 32*24 chars (256 x 192 pixels)";
    zxDialog->printText(reinterpret_cast<uint8_t *>(bcmFrameBuffer->GetBuffer()), 1, 31, 0xC, 0x0, innerDimensions);
    char const *outerDimensions = "Outer: 44*34 chars (352 x 272 pixels)";
    zxDialog->printText(reinterpret_cast<uint8_t *>(bcmFrameBuffer->GetBuffer()), 1, 32, 0xE, 0x0, outerDimensions);

    for (unsigned int y = 0; y < 34; y++) {
        char rowNum[10] = {};
        sprintf(rowNum, "%02d", y);
        zxDialog->printText(reinterpret_cast<uint8_t *>(bcmFrameBuffer->GetBuffer()), 41, y, 0xD, 0x0, rowNum);
    }
#endif

    /* Read the Raspberry Pi framebuffer and turn it into a QImage object.
     *
     * Image data in the framebuffer is stored linearly starting with pixel (0,0) on the top left and the moving right
     * and down one pixel at a time.  In our particular example we have 2 pixels per byte and the bytes are out of
     * sequence because they are stored as a single 32 bit unsigned integer instead of 4 individual bytes.
     */
#if BYTE_SWAP_DISABLED
    for (int i = 0; i < 272; i++) {
        for (int j = 0; j < 44; j++) {
            for (int k=0; k<4; k++) {
                uint8_t byte = videoMemory.at( (i * 176) + (j * 4) + (3 - k) );
                raspberryPiImage.setPixel( (j * 8) + (k * 2), i, bcmFrameBuffer->palette[(byte & 0xF0) >> 4] );
                raspberryPiImage.setPixel( (j * 8) + (k * 2) + 1 , i, bcmFrameBuffer->palette[byte & 0x0F] );
            }
        }
    }
#else
    for (int i = 0x0000; i < 0x0110; i++) {
        for (int j = 0x0000; j < 0x00B0; j++) {
            uint8_t byte = reinterpret_cast<char *>(bcmFrameBuffer->GetBuffer())[(i * 0x00B0) + j];
            raspberryPiImage.setPixel((j * 2), i, bcmFrameBuffer->palette[(byte & 0xF0u) >> 0x04u]);
            raspberryPiImage.setPixel((j * 2) + 1, i, bcmFrameBuffer->palette[byte & 0x0Fu]);
        }
    }
#endif

    /* In theory, the main screen should have a border of 48 pixels on the left,  48 on the right, 48 on top and 56
     * pixels at the bottom but the ZX Spectrum bare emulator only seems to have a 32 pixel border at the top and a 48
     * pixel border at the bottom.
     * 
     * The left margin also appears to be smaller than the right margin on the Raspberry Pi.
     * 
     * http://www.zxdesign.info/vidparam.shtml
     */
    painter.drawImage(QRect(0, 0, 352 * scale, 272 * scale), raspberryPiImage);

    painter.restore();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
}
