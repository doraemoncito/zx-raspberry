/*
 * Copyright (c) 2020-2022 Jose Hernandez
 * Copyright (c) 2017 José Luis Sanchez
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
#include <cassert>
#include <cstdint>
#include <cstring>
#include <circle/bcmframebuffer.h>
#include <circle/logger.h>
#include <circle/util.h>
#include "zxdisplay.h"
#include "gui/zxview.h"


static const char msgFromDisplay[] = "Display";


ZxDisplay::ZxDisplay()
        : m_pZxView(nullptr),
          m_pFrameBuffer(nullptr),
          m_pVideoMem(nullptr),
          m_border(0xFu),
          m_bBorderChanged(false),
          m_bDoubleBufferingEnabled(false),
          m_bVSync(false),
          m_bBufferSwapped(false),
          m_pBaseBuffer(nullptr),
          m_pBuffer(nullptr),
          m_lastBorderChanged(0) {
}


ZxDisplay::~ZxDisplay() {
    // Delete the video frame buffer
    delete m_pFrameBuffer;

    // Delete the colour / screen lookup table
    delete[] m_pScrTable;

    m_pVideoMem = nullptr;
    m_pFrameBuffer = nullptr;
    m_pBaseBuffer = nullptr;
    m_pBuffer = nullptr;
}


bool ZxDisplay::Initialize(uint8_t *pVideoMem, CBcmFrameBuffer *pFrameBuffer) {

    // lookup table
    m_pScrTable = reinterpret_cast<uint32_t (*)[256][256]>(new uint32_t[256][256]);

    m_pVideoMem = pVideoMem;
    assert(m_pVideoMem != nullptr);

    m_pFrameBuffer = pFrameBuffer;
    assert(m_pFrameBuffer != nullptr);

    /* Set-up the colour palette definition in RGB565 format.
     * Needs to be defined BEFORE the call to initialize the framebuffer.
     */
    for (unsigned long i = 0; i < (sizeof(m_palette)/sizeof(uint16_t)); i++) {
        m_pFrameBuffer->SetPalette(i, m_palette[i]);
    }

    if (!m_pFrameBuffer->Initialize()) {
        return false;
    }

    m_pBaseBuffer = m_pTargetBuffer8 = reinterpret_cast<uint8_t *>(m_pFrameBuffer->GetBuffer());
    m_pBuffer = m_pBaseBuffer + SCREEN_WIDTH * (SCREEN_HEIGHT / 2);

    std::memset(m_pBaseBuffer, ((m_border << 0x4u) | m_border), m_pFrameBuffer->GetSize());

    /*
     * Create a lookup table for draw the screen Faster Than Light :)
     *
     * The attribute byte format is as follows:
     *
     *  | F | B | P2 | P1 | P0 | I2 | I1 | I0 |
     *
     * F sets the attribute FLASH mode
     * B sets the attribute BRIGHTNESS mode
     * P2 to P0 is the PAPER colour
     * I2 to I0 is the INK colour
     */
    for (int attr = 0; attr < 128; attr++) {
        unsigned ink = attr & 0x07u;
        unsigned paper = (attr & 0x78u) >> 3;
        // Brighten up the ink colour if the brightness bit is on
        if (attr & 0x40) {
            ink |= 0x08;
        }
        for (int idx = 0; idx < 256; idx++) {
            for (unsigned mask = 0x80; mask > 0; mask >>= 1) {
                *m_pScrTable[attr][idx] <<= 4;
                *m_pScrTable[attr + 128][idx] <<= 4;
                if (idx & mask) {
                    *m_pScrTable[attr][idx] |= ink;
                    *m_pScrTable[attr + 128][idx] |= paper;
                } else {
                    *m_pScrTable[attr][idx] |= paper;
                    *m_pScrTable[attr + 128][idx] |= ink;
                }
            }
            *m_pScrTable[attr][idx] = bswap32(*m_pScrTable[attr][idx]);
            *m_pScrTable[attr + 128][idx] = bswap32(*m_pScrTable[attr + 128][idx]);
        }
    }

    return true;
}


void ZxDisplay::update(bool flash) {

    assert(m_pBaseBuffer != nullptr);
    assert(m_pBuffer != nullptr);
    assert(m_pVideoMem != nullptr);

    if (m_bDoubleBufferingEnabled) {
        if (m_bVSync) {
            // In VSync mode we swap the target frame each we refresh the screen
            m_pFrameBuffer->SetVirtualOffset(0, m_bBufferSwapped ? SCREEN_HEIGHT : 0);
            m_pFrameBuffer->WaitForVerticalSync();
            m_bBufferSwapped = !m_bBufferSwapped;
        } else {
            // In non-vsync mode we just copy from the hidden buffer to the base buffer
            memcpy(m_pBaseBuffer, m_pBuffer, (SCREEN_WIDTH * SCREEN_HEIGHT) / 2 /* 2 pixels per byte */);
        }
    }

    /*
     * Determine the location of the next target buffer.  If double buffering is not enabled, the next target buffer
     * will always be the base buffer.
     */
    m_pTargetBuffer8 = (m_bDoubleBufferingEnabled && m_bBufferSwapped) ? m_pBuffer : m_pBaseBuffer;
    auto pTargetBuffer32 = reinterpret_cast<uint32_t *>(m_pTargetBuffer8);

    /*
     * Calculate the index into the frame buffer to perform fast translation of ZX Spectrum video memory to Raspberry Pi
     * framebuffer memory.  The framebuffer pointer has 32 bits per element. e.g 8 pixels since each pixel takes 4 bits.
     *
     *      48 lines * 352 pixels per line + 48 border pixels = 16944 pixels
     *      16944 pixels offset / 8 pixels per array element = 2118 array index = 0x0846 HEX
     */
    int bufIdx = 0x0846;

    // Offset into the ZX Spectrum colour attribute memory (6144 bytes)
    int attribute = 0x1800;

    static uint8_t flashMask[] = {0x7Fu, 0xFFu};

    // Always force a border repaint if double buffering is enabled.
//    m_bBorderChanged = m_bDoubleBufferingEnabled;

    // Draw the border
//    if (m_bBorderChanged) {
//        std::memset(pTargetBuffer8, ((m_border << 0x4u) | m_border), (SCREEN_WIDTH * SCREEN_HEIGHT) / 2 /* 2 pixels per byte */);
//        m_bBorderChanged = false;
//    }

    // The ZX Spectrum screen is made up of 3 blocks of 2048 (0x0800) bytes each
    for (unsigned int block = 0x0000; block < 0x1800; block += 0x0800) {
        for (unsigned int row = 0x0000; row < 0x0100; row += 0x0020) {
            for (unsigned int column = 0x0000; column < 0x0020; column++) {
                uint8_t colour = m_pVideoMem[attribute++] & flashMask[flash];
                pTargetBuffer32[bufIdx + column + 0x0000] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0000]];
                pTargetBuffer32[bufIdx + column + 0x002C] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0100]];
                pTargetBuffer32[bufIdx + column + 0x0058] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0200]];
                pTargetBuffer32[bufIdx + column + 0x0084] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0300]];
                pTargetBuffer32[bufIdx + column + 0x00B0] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0400]];
                pTargetBuffer32[bufIdx + column + 0x00DC] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0500]];
                pTargetBuffer32[bufIdx + column + 0x0108] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0600]];
                pTargetBuffer32[bufIdx + column + 0x0134] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0700]];
            }
            bufIdx += 0x0160;
        }
    }

    if (m_pZxView != nullptr) {
        m_pZxView->draw(m_pTargetBuffer8);
    }

    m_lastBorderChanged = 0;
}


void ZxDisplay::setBorder(uint8_t border) {

    if (this->m_border != border) {
        this->m_bBorderChanged = true;
        this->m_border = border;
    }
}


void ZxDisplay::updateBorder(uint8_t border, uint32_t tstates) {

    uint8_t colour = (border << 0x4u) | border;

    // FOR TESTING ONLY
    for (unsigned int row = 0x0000; row < 0x0B00; row += 0x0B0) {
        for (unsigned int column = 0x0000; column < 0x0008; column++) {
            m_pTargetBuffer8[row + column] = colour;
        }
    }

    CLogger::Get()->Write(msgFromDisplay, LogDebug,
                          "[update border] m_lastBorderChanged: %d, portFE: 0x%02X, T-States: %d",
                          m_lastBorderChanged, border, tstates);

    if (m_lastBorderChanged < tstates) {

        tstates &= 0X00FFFFFCu;
        while (m_lastBorderChanged < tstates) {
// TODO: work out the position to draw at based on the number of states elapsed.
//            int position = m_states2border[m_lastBorderChanged];
            m_lastBorderChanged += 4;

//            if ((m_lastBorderChanged != 0XF0CAB0BAu) && (m_pTargetBuffer8[m_lastBorderChanged] != colour)) {
//                // draw the border, 8 pixels (or 4 bytes) at a time
//                for (int i = 0; i < 4; i++) {
//                    m_pTargetBuffer8[m_lastBorderChanged + i] = colour;
//                }
//            }
        }

        m_lastBorderChanged = tstates;
    }
}


void ZxDisplay::setUI(ZxView *pZxView) {

    this->m_pZxView = pZxView;
}
