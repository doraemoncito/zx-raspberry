/*
 * Copyright (c) 2020-2024 Jose Hernandez
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
          m_border(0x07u),
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

    m_pVideoMem = pVideoMem;
    assert(m_pVideoMem != nullptr);

    m_pFrameBuffer = pFrameBuffer;
    assert(m_pFrameBuffer != nullptr);

    /* Set up the colour palette definition in RGB565 format.
     * Needs to be defined BEFORE the call to initialise the framebuffer.
     */
    for (uint32_t i = 0; i < (sizeof(m_palette)/sizeof(uint16_t)); i++) {
        CLogger::Get()->Write("[Display]", LogDebug,"Setting palette index %02d to '%s' (RGB565: 0x%04X)",
                              i, m_paletteColourName[i], m_palette[i]);
        m_pFrameBuffer->SetPalette(i, m_palette[i]);
    }

    if (!m_pFrameBuffer->Initialize()) {
        return false;
    }

    m_pBaseBuffer = m_pTargetBuffer8 = reinterpret_cast<uint8_t *>(m_pFrameBuffer->GetBuffer());
    m_pBuffer = m_pBaseBuffer + DISPLAY_WIDTH * (DISPLAY_HEIGHT / 2);
    m_pTargetBuffer32 = reinterpret_cast<uint32_t *>(m_pTargetBuffer8);

    // Initialise the screen
    std::memset(m_pBaseBuffer, static_cast<int>((m_border << 0x04u) | m_border), m_pFrameBuffer->GetSize());

    /*
     * Create a pixel value lookup table to draw the screen as fast we possibly can. This section was adapted from
     * sample code by José Luis Sanchez of ZXBaremulator (https://zxmini.speccy.org/en/index.html) fame.
     *
     * The attribute byte format is as follows:
     *
     *  | F | B | P2 | P1 | P0 | I2 | I1 | I0 |
     *
     * F sets the attribute FLASH mode where flashing is done by swapping the ink and paper colours
     * B sets the attribute BRIGHTNESS mode
     * P2 to P0 is the PAPER colour
     * I2 to I0 is the INK colour
     *
     * The lookup table maps an attribute byte and a character (8 pixel) mask byte into a 32-bit value where each
     * nibble represents a 4-bit depth pixel colour on screen. If the colour depth was to change, we would need to
     * amend the way we create this lookup table.
     *
     * To look up a cached value, we need to perform the following operation:
     *
     *      uint8_t colour = attribute & (flash) ? 0xFFu : 0x7Fu;
     *      uint32_t pixels = *m_pScrTable[colour][character];
     */
    m_pScrTable = reinterpret_cast<uint32_t (*)[256][256]>(new uint32_t[256 * 256]);

    /* Iterate over all the possible attribute values...
     *
     * We compute all the normal attributes (cache table position 0..127) followed by the flashed attributes
     * (cache table position 128..255).
     */
    for (uint32_t attr = 0; attr < 256; attr++) {

        /* The ink and paper values are unpacked like this:
         *
         *  ink:   | F | B | P2 | P1 | P0 | I2 | I1 | I0 |  -->  | 0 | 0 | 0 | 0 | B | I2 | I1 | I0 |
         *  paper: | F | B | P2 | P1 | P0 | I2 | I1 | I0 |  -->  | 0 | 0 | 0 | 0 | B | P2 | P1 | P0 |
         */
        uint32_t ink = (attr & 0b01000000) >> 3 | (attr & 0b00000111);
        uint32_t paper = (attr & 0b01111000) >> 3;
        uint32_t flash = attr >> 7;

        // Each 8-bit character mask can take on 256 possible values from 0 to 255.
        for (uint32_t character = 0; character < 256; character++) {
            /* Iterate over each of the 8 pixels in the character, shifting the target value by one nibble to the
             * left in each iteration. Then apply the ink or paper value to the rightmost nibble depending on
             * whether the character pixel is on or off.
             */
            for (uint32_t mask = 0b10000000; mask > 0; mask >>= 1) {
                *m_pScrTable[attr][character] <<= 4;
                *m_pScrTable[attr][character] |= flash ? ((character & mask) ? paper : ink) : ((character & mask) ? ink : paper);
            }
            // Swap the 32 bit cached value depending on whether the machine is big or little endian.
            *m_pScrTable[attr][character] = bswap32(*m_pScrTable[attr][character]);
        }
    }

    /* Pre-compute the border fill value used to set 8 pixels (e.g. 4 bytes when working in 4-bit depth) at a time.
     * This is effectively 4 bytes where all 8 nibbles (4 bits) are identical.
     * Border colours do not use brightness.
     */
    for (uint8_t border = 0; border < 8; border++) {
        for (uint8_t nibble = 0; nibble < 8; nibble++) {
            m_borderColour[border] = (m_borderColour[border] << 4) | border;
        }
    }

    //m_firstBorderUpdate = ((64 - screenGeometry.border().top()) * spectrumModel.tstatesLine) - screenGeometry.border().left() / 2;
    m_firstBorderUpdate = ((64 - TOP_BORDER) * 224) - (LEFT_BORDER / 2);
    //m_lastBorderUpdate = (255 + BOTTOM_BORDER) * Clock::getInstance().getTstatesPerScreenLine() + 128 + RIGHT_BORDER;
    m_lastBorderUpdate = ((255 + BOTTOM_BORDER) * 224) + 128 + RIGHT_BORDER;
    CLogger::Get()->Write("[Display]", LogDebug," m_lastBorderUpdate: %d", m_lastBorderUpdate);

    return true;
}


void ZxDisplay::update(bool flash) {

    assert(m_pBaseBuffer != nullptr);
    assert(m_pBuffer != nullptr);
    assert(m_pVideoMem != nullptr);

//    CLogger::Get()->Write("[Display]", LogDebug,"(update display) Frame: %5d; T-states: %5d",
//                          Clock::getInstance().getFrames(), Clock::getInstance().getTstates());

    if (m_bDoubleBufferingEnabled) {
        if (m_bVSync) {
            // In VSync mode we swap the target frame each we refresh the screen
            m_pFrameBuffer->SetVirtualOffset(0, m_bBufferSwapped ? DISPLAY_HEIGHT : 0);
            m_pFrameBuffer->WaitForVerticalSync();
            m_bBufferSwapped = !m_bBufferSwapped;
        } else {
            m_pFrameBuffer->SetVirtualOffset(0, DISPLAY_HEIGHT);
            // In non-vsync mode, we just copy from the hidden buffer to the base buffer
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
     * framebuffer memory. The framebuffer pointer has 32 bits per element, e.g. 8 pixels since each pixel takes 4 bits.
     *
     *      48 lines * 352 pixels per line + 48 border pixels = 16944 pixels
     *      16944 pixels offset / 8 pixels per array element = 2118 array index = 0x0846 HEX
     */
    int bufIdx = 0x0846;

    // Offset into the ZX Spectrum colour attribute memory (6144 bytes)
    int attribute = 0x1800;

    static uint8_t flashMask[] = {0x7Fu, 0xFFu};

    updateBorder(m_border, m_lastBorderUpdate);

    // BEGIN DEBUG (place a flashing checkered box in the top right corner of the spectrum video memory)
//    uint32_t xblock = 0x0000;
//    uint32_t xcolumn = 0x001F;
//    uint32_t xrow = 0x0000;
//    for (uint32_t xline = 0; xline < 8; xline++) {
//        m_pVideoMem[xblock + xrow + xcolumn + xline * 0x0100] = flash
//                ? ((xline % 2 == 0) ? 0xAA : 0x55)
//                : ((xline % 2 == 0) ? 0x55 : 0xAA);
//    }
//    m_pVideoMem[attribute + xcolumn] = flash ? 0xAA : 0x55;
    // END DEBUG

    // The ZX Spectrum screen is made up of 3 blocks of 2048 (0x0800) bytes each
    for (uint32_t block = 0x0000; block < 0x1800; block += 0x0800) {
        for (uint32_t row = 0x0000; row < 0x0100; row += 0x0020) {
            for (uint32_t column = 0x0000; column < 0x0020; column++) {
                uint8_t colour = m_pVideoMem[attribute++] & flashMask[flash];
                for (uint32_t line = 0; line < 8; line++) {
                    m_pTargetBuffer32[bufIdx + column + line * 0x2C] = *m_pScrTable[colour][m_pVideoMem[block + row + column + line * 0x0100]];
                }
            }
            bufIdx += 0x0160;
        }
    }

//    // BEGIN DEBUG
//    auto label = new ZxLabel(ZxRect(1, 1, 1, 1), (flash ? "O" : "X"));
//    label->draw(m_pTargetBuffer8);
//    delete label;
//    // END DEBUG

    if (m_pZxView != nullptr) {
        m_pZxView->draw(m_pTargetBuffer8);
    }

    m_lastBorderChange = m_firstBorderUpdate;
}


/*
 * Each full line on the display takes 224 T-states to draw, where each line can be broken into the following
 * substates:
 *
 *      128 T-states to draw 256 screen pixels
 *       24 T-states to draw the 48 right border pixels
 *       48 T-states for the horizontal retrace
 *       24 T-states to draw the 48 left border pixels
 *
 * Each complete ZX Spectrum screen has 312 lines divided into:
 *
 *       16 lines to allow the electron beam to return to the top of the screen
 *       48 lines for the top border
 *      192 lines to draw the 192 screen pixels
 *       56 lines for the bottom border (out of which only 48 lines are actually visible in the real machine)
 */
void ZxDisplay::updateBorder(uint8_t border, uint32_t tstates) {

//    CLogger::Get()->Write("[Display]", LogDebug,
//                          "(update border #1) m_lastBorderChange: %5d; T-states: %5d; portFE: %d (%s)",
//                          m_lastBorderChange, tstates, m_border, m_paletteColourName[m_border & 0x07],
//                          tstates, border, m_paletteColourName[border & 0x07]);

    if ((tstates >= m_lastBorderChange) && (m_lastBorderChange <= m_lastBorderUpdate)) {

//        CLogger::Get()->Write("[Display]", LogDebug,
//                              "(update border #2) m_lastBorderChange: %5d; T-states: %5d; portFE: %d (%s)",
//                              m_lastBorderChange, tstates, m_border, m_paletteColourName[m_border & 0x07],
//                              tstates, border, m_paletteColourName[border & 0x07]);

        auto clock = Clock::getInstance();

        // Draw the border 8 pixels (e.g. 4 bytes) at a time
        while (m_lastBorderChange < tstates) {

            uint32_t offset = (m_lastBorderChange - 176) % clock.getTstatesPerScreenFrame();
            uint32_t row = offset / clock.getTstatesPerScreenLine();
            uint32_t col = offset % clock.getTstatesPerScreenLine();

            //CLogger::Get()->Write("[ZxDisplay]", LogDebug,"[BORDER] row: %3d, column: %3d, border: 0x%02X, colour: %-14s", row, col, m_border, m_paletteColourName[border]);

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
                m_pTargetBuffer32[baseAddress] = m_borderColour[m_border];
            }
            m_lastBorderChange += 4;
        }

    }

    m_lastBorderChange = tstates;
    m_border = border;
}


void ZxDisplay::setUI(ZxView *pZxView) {

    this->m_pZxView = pZxView;
}
