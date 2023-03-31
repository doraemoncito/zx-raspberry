#include <QPainter>
#include <QtGlobal>
#include <QtDebug>
#include "../common/BruceLeeScr.h"
#include "screen.h"


int scale = 2;

Screen::Screen(QWidget *parent) : QWidget(parent)
{
    antiAliased = false;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    bcmFrameBuffer = new CBcmFrameBuffer(352, 272, 4);
    m_zxDisplay.Initialize(BruceLee_scr, bcmFrameBuffer);
}

QSize Screen::minimumSizeHint() const
{
    return QSize(scale * 2 * 352 /* 48 + 256 + 48 */, scale * 296 /*(48 + 192 + 56)*/);
}

QSize Screen::sizeHint() const
{
    return QSize(scale * 2 * 352 /* 48 + 256 + 48 */, scale * 296 /*(48 + 192 + 56)*/);
}

void Screen::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, antiAliased);
    painter.save();
    m_zxDisplay.update(false);

    // Reads ZX Spectrum video memory and turns it into a QImage object. 
    for (int section = 0; section < 3; section++) {                    // 192 pixels divided into 3 sections of
        for (int character = 0; character < 8; character++) {          // 8 rows of characters
            for (int row = 0; row < 8; row++) {                        // where each charater is 8 pixels heigh
                int srcRow = (section * 64) + (character * 8) + row;
                int dstRow = (section * 64) + character + (row * 8);
                // qDebug("Translating row: %d to %d", srcRow, dstRow);

                for (int col = 0; col < 32; col++) {                   // 32 columns, each 8 pixel wide = 256 pixels

                    // 8 pixel monochrome data byte representing a simple on or off state.
                    // The pixel colour itself will come from the attribute map.  
                    uint8_t byte = BruceLee_scr[srcRow * 32 + col];

                    /* Attribute RAM is located at an offet of 0x1800 (6144) bytes from the start of
                     * video display RAM.
                     * 
                     * http://www.overtakenbyevents.com/lets-talk-about-the-zx-specrum-screen-layout/
                     */
                    uint8_t attr = BruceLee_scr[0x1800 + (dstRow / 8) * 32 + col];
                    uint8_t ink = ((attr & 0x40) >> 3) | (attr & 0x07);
                    uint8_t paper = (attr & 0x78) >> 3; // (((attr & 0x40) >> 3) | (attr & 0x38) >> 3)

                    // qDebug("Position (%d, %d), INK: 0x%1X (0x%08X), PAPER: 0x%1X (0x%08X)", col, srcRow, ink, colours[ink], paper, colours[paper]);

                    for (uint8_t bit=0; bit<8; bit++) {
                        /* When the target pixel is on we use the ink colour otherwise we use the paper colour the the
                         * target pixel is off.
                         * 
                         * http://www.overtakenbyevents.com/lets-talk-about-the-zx-specrum-screen-layout-part-two/
                         */
                        zxSpectrumImage.setPixel(col * 8 + 8 - bit - 1 , dstRow, bcmFrameBuffer->palette[((byte >> bit) & 0x1) ? ink : paper]);
                    }
                }
            }
        }
    }

    painter.drawImage(QRect(48 * scale, 48 * scale, 256 * scale, 192 * scale), zxSpectrumImage);

    QByteArray videoMemory = QByteArray::fromRawData((const char *) bcmFrameBuffer->GetBuffer(), bcmFrameBuffer->GetSize());

    /* Reads Raspberry Pi framebuffer and turns it into a QImage object.
     *
     * Image data in the framebuffer is stored linearly starting with pixel (0,0)
     * on the top left and the moving right and down one pixel at a time.  In our
     * particular example we have 2 pixels per byte and the bytes are out of
     * sequence because they are stored was 32 bit unsigned integers in one go
     * insetad of 4 individual bytes.
     */
#if BYTE_SWAP_DISABLED
    for (int i = 0; i < 272; i++) {
        for (int j = 0; j < 44; j++) {
            for (int k=0; k<4; k++) {
                uint8_t byte = videoMemory.at( (i * 176) + (j * 4) + (3 - k) );
                raspberryPiImage.setPixel( (j * 8) + (k * 2), i, m_pBcmFrameBuffer->palette[(byte & 0xF0) >> 4] );
                raspberryPiImage.setPixel( (j * 8) + (k * 2) + 1 , i, m_pBcmFrameBuffer->palette[byte & 0x0F] );
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

    /* In theory, the main screen should have a border of 48 pixels on the left,
     * 48 on the right, 48 on top and 56 pixels at the bottom but the ZX Spectrum
     * bare emulator only seems to have a 32 pixel border at the top and a 48
     * pixel border at the bottom.
     * 
     * The left margin also appers to be smaller than the right margin on the
     * Raspeberry Pi.
     * 
     * http://www.zxdesign.info/vidparam.shtml
     */
    painter.drawImage(QRect(352 * scale, 0, 352 * scale, 272 * scale), raspberryPiImage);

    painter.restore();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
}
