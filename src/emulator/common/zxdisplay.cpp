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
#include <circle/util.h>
#include "zxdisplay.h"


ZxDisplay::ZxDisplay()
        : m_pFrameBuffer(nullptr),
          m_pBuffer(nullptr),
          m_pVideoMem(nullptr),
          m_border(0xFu),
          m_borderChanged(false) {
}


ZxDisplay::~ZxDisplay() {
    // Delete the video frame buffer
    delete m_pFrameBuffer;

    // Delete the colour / screen lookup table
    delete[] m_pScrTable;

    m_pVideoMem = nullptr;
    m_pBuffer = nullptr;
    m_pFrameBuffer = nullptr;
}


bool ZxDisplay::Initialize(uint8_t *pVideoMem, CBcmFrameBuffer *pFrameBuffer) {

    // lookup table
    m_pScrTable = reinterpret_cast<uint32_t (*)[256][256]>(new uint32_t[256][256]);

    m_pVideoMem = pVideoMem;
    assert(m_pVideoMem != nullptr);

    m_pFrameBuffer = pFrameBuffer;
    assert(m_pFrameBuffer != nullptr);

    // Color palette definition in RGB565 format.
    // Needs to be defined BEFORE the call to initialize the framebuffer.
    m_pFrameBuffer->SetPalette(0x0, 0x0000u); // black
    m_pFrameBuffer->SetPalette(0x1, 0x0010u); // blue
    m_pFrameBuffer->SetPalette(0x2, 0x8000u); // red
    m_pFrameBuffer->SetPalette(0x3, 0x8010u); // magenta
    m_pFrameBuffer->SetPalette(0x4, 0x0400u); // green
    m_pFrameBuffer->SetPalette(0x5, 0x0410u); // cyan
    m_pFrameBuffer->SetPalette(0x6, 0x8400u); // yellow
    m_pFrameBuffer->SetPalette(0x7, 0x8410u); // white
    m_pFrameBuffer->SetPalette(0x8, 0x0000u); // black
    m_pFrameBuffer->SetPalette(0x9, 0x001Fu); // bright blue
    m_pFrameBuffer->SetPalette(0xA, 0xF800u); // bright red
    m_pFrameBuffer->SetPalette(0xB, 0xF81Fu); // bright magenta
    m_pFrameBuffer->SetPalette(0xC, 0x07E0u); // bright green
    m_pFrameBuffer->SetPalette(0xD, 0x07FFu); // bright cyan
    m_pFrameBuffer->SetPalette(0xE, 0xFFE0u); // bright yellow
    m_pFrameBuffer->SetPalette(0xF, 0xFFFFu); // bright white

    if (!m_pFrameBuffer->Initialize()) {
        return false;
    }

    m_pBuffer = reinterpret_cast<uint32_t *>(m_pFrameBuffer->GetBuffer());
    assert(m_pBuffer != nullptr);

    std::memset(m_pBuffer, ((m_border << 0x4u) | m_border), m_pFrameBuffer->GetSize());

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
        unsigned ink = attr & 0x07;
        unsigned paper = (attr & 0x78) >> 3;
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

    assert(m_pBuffer != nullptr);
    assert(m_pVideoMem != nullptr);

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

    // TODO: eliminate branching to allow this code to run efficiently on the Raspberry Pi's GPU
    if (m_borderChanged) {
        std::memset(m_pBuffer, ((m_border << 0x4u) | m_border), m_pFrameBuffer->GetSize());
        m_borderChanged = false;
    }

    // The ZX Spectrum screen is made up of 3 blocks of 2048 (0x0800) bytes each
    for (unsigned int block = 0x0000; block < 0x1800; block += 0x0800) {
        for (unsigned int row = 0x0000; row < 0x0100; row += 0x0020) {
            for (unsigned int column = 0x0000; column < 0x0020; column++) {
                uint8_t colour = m_pVideoMem[attribute++] & flashMask[flash];
                m_pBuffer[bufIdx + column + 0x0000] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0000]];
                m_pBuffer[bufIdx + column + 0x002C] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0100]];
                m_pBuffer[bufIdx + column + 0x0058] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0200]];
                m_pBuffer[bufIdx + column + 0x0084] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0300]];
                m_pBuffer[bufIdx + column + 0x00B0] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0400]];
                m_pBuffer[bufIdx + column + 0x00DC] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0500]];
                m_pBuffer[bufIdx + column + 0x0108] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0600]];
                m_pBuffer[bufIdx + column + 0x0134] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0700]];
            }
            bufIdx += 0x0160;
        }
    }
}


void ZxDisplay::setBorder(uint8_t border) {

    if (this->m_border != border) {
        this->m_borderChanged = true;
        this->m_border = border;
    }
}
