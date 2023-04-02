/*
 * Copyright (c) 2020-2023 Jose Hernandez
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
#include "clock.h"


ZxDisplay::ZxDisplay()
        : m_pZxView(nullptr),
          m_pFrameBuffer(nullptr),
          m_pVideoMem(nullptr),
          m_border(0xFu),
          m_bDoubleBufferingEnabled(false),
          m_bVSync(false),
          m_bBufferSwapped(false),
          m_pBaseBuffer(nullptr),
          m_pBuffer(nullptr),
          m_lastBorderChange(0) {
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

    /* Set up the colour palette definition in RGB565 format.
     * Needs to be defined BEFORE the call to initialize the framebuffer.
     */
    for (unsigned long i = 0; i < (sizeof(m_palette)/sizeof(uint16_t)); i++) {
        m_pFrameBuffer->SetPalette(i, m_palette[i]);
    }

    if (!m_pFrameBuffer->Initialize()) {
        return false;
    }

    m_pBaseBuffer = m_pTargetBuffer8 = reinterpret_cast<uint8_t *>(m_pFrameBuffer->GetBuffer());
    m_pBuffer = m_pBaseBuffer + DISPLAY_WIDTH * (DISPLAY_HEIGHT / 2);
    m_pTargetBuffer32 = reinterpret_cast<uint32_t *>(m_pTargetBuffer8);

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

    // Pre-compute the border fill word use to set 8 pixels (i.e. 4 bytes) at a time
    for (int border = 0; border < 8; border++) {
        for (int halfByte = 0; halfByte < 8; halfByte++) {
            m_colour[border] = (m_colour[border] << 0x4u) | border;
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
            m_pFrameBuffer->SetVirtualOffset(0, m_bBufferSwapped ? DISPLAY_HEIGHT : 0);
            m_pFrameBuffer->WaitForVerticalSync();
            m_bBufferSwapped = !m_bBufferSwapped;
        } else {
            m_pFrameBuffer->SetVirtualOffset(0, DISPLAY_HEIGHT);
            // In non-vsync mode we just copy from the hidden buffer to the base buffer
            memcpy(m_pBaseBuffer, m_pBuffer, (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2 /* 2 pixels per byte */);
        }
    }

    /*
     * Determine the location of the next target buffer.  If double buffering is not enabled, the next target buffer
     * will always be the base buffer.
     */
    m_pTargetBuffer8 = (m_bDoubleBufferingEnabled && m_bBufferSwapped) ? m_pBuffer : m_pBaseBuffer;
    m_pTargetBuffer32 = reinterpret_cast<uint32_t *>(m_pTargetBuffer8);

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

    updateBorder(m_border, Clock::getInstance().getTstatesPerScreenFrame());

    // The ZX Spectrum screen is made up of 3 blocks of 2048 (0x0800) bytes each
    for (unsigned int block = 0x0000; block < 0x1800; block += 0x0800) {
        for (unsigned int row = 0x0000; row < 0x0100; row += 0x0020) {
            for (unsigned int column = 0x0000; column < 0x0020; column++) {
                uint8_t colour = m_pVideoMem[attribute++] & flashMask[flash];
                m_pTargetBuffer32[bufIdx + column + 0x0000] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0000]];
                m_pTargetBuffer32[bufIdx + column + 0x002C] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0100]];
                m_pTargetBuffer32[bufIdx + column + 0x0058] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0200]];
                m_pTargetBuffer32[bufIdx + column + 0x0084] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0300]];
                m_pTargetBuffer32[bufIdx + column + 0x00B0] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0400]];
                m_pTargetBuffer32[bufIdx + column + 0x00DC] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0500]];
                m_pTargetBuffer32[bufIdx + column + 0x0108] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0600]];
                m_pTargetBuffer32[bufIdx + column + 0x0134] = *m_pScrTable[colour][m_pVideoMem[block + row + column + 0x0700]];
            }
            bufIdx += 0x0160;
        }
    }

    if (m_pZxView != nullptr) {
        m_pZxView->draw(m_pTargetBuffer8);
    }

    m_lastBorderChange = 0;
}


/*
 * Each full line on the display takes 224 T-states to draw, where each line can be broken into the following
 * substates:
 *
 *      128 T-states to draw the 256 screen pixels
 *       24 T-states to draw the 48 right border pixels
 *       48 T-states for the horizontal retrace
 *       24 T-states to draw the 48 left border pixels
 *
 * Each complete ZX spectrum screen has 312 lines divided into:
 *
 *       16 lines to allow the electron beam to return to the top of the screen
 *       48 lines for the top border
 *      192 lines to draw the 192 screen pixels
 *       56 lines for the bottom border (out of which only 48 lines are actually visible in the real machine)
 */
void ZxDisplay::updateBorder(uint8_t border, uint32_t tstates) {

    if (m_border != border) {
        CLogger::Get()->Write("[ZxDisplay]", LogDebug,
                              "[BORDER] m_lastBorderChange: %5d, T-States: %5d, portFE: 0x%02X, colour: %-14s",
                              m_lastBorderChange, tstates, border, m_borderColourName[border]);
    }

    auto clock = Clock::getInstance();

    // Draw the border 8 pixels (e.g. 4 bytes) at a time
    while (m_lastBorderChange < tstates) {

        uint32_t offset = m_lastBorderChange % clock.getTstatesPerScreenFrame();
        uint32_t row = offset / clock.getTstatesPerScreenLine();
        uint32_t col = offset % clock.getTstatesPerScreenLine();

        //CLogger::Get()->Write("[ZxDisplay]", LogDebug,"[BORDER] row: %3d, column: %3d, border: 0x%02X, colour: %-14s", row, col, m_border, m_borderColourName[border]);

        /*
         * Determine whether the current T-state falls within the border area and paint it using the cached border
         * fill colour if so.
         */
        if ((col < 24) || (col >= 152 && col < 176) || (row < 48) || (row >= 240 && row < 296)) {
            /*
             * When calculating the offset into the screen buffer we need to take into account the 48 T-states used to
             * return the electron beam to the start of the line and divide both the column and the row by 4 (bytes)
             * since we are drawing 8 pixels at a time.
             */
            uint32_t baseAddress = row * ((clock.getTstatesPerScreenLine() - 48) / 4) + (col / 4);
            m_pTargetBuffer32[baseAddress] = m_colour[m_border];
        }
        m_lastBorderChange += 4;
    }

    m_lastBorderChange = tstates;
    m_border = border;
}


void ZxDisplay::setUI(ZxView *pZxView) {

    this->m_pZxView = pZxView;
}
