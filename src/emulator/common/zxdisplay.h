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
#ifndef ZXDISPLAY_H
#define ZXDISPLAY_H

#include <cstdint>
#include <circle/bcmframebuffer.h>
#include <circle/types.h>

class ZxView;

class ZxDisplay {
public:
    ZxDisplay();
    ~ZxDisplay();

    bool Initialize(uint8_t *pVideoMem, CBcmFrameBuffer *pFrameBuffer);
    void update(bool flash);
    void updateBorder(uint8_t portFE, uint32_t tstates);

    void setUI(ZxView *pZxView);
    ZxView *getUI() {
        return m_pZxView;
    };

    /* These are the visible screen dimensions, which are smaller than the actual dimensions suggested by the
     * screen timings since it takes the electron beam some time to fly back to the beginning or top of the screen:
     *
     * WIDTH:  48 pixel border + 256 pixel screen + 48 pixel border = 352 pixels = 44 characters
     * HEIGHT: 48 pixel border + 192 pixel screen + 56 pixel border = 296 pixels = 37 characters
     *
     * Follow these links for more information on the screen layout and timings:
     *
     * - <http://www.zxdesign.info/vidresearch.shtml>
     * - <http://www.zxdesign.info/vidparam.shtml>
     * - <http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout>
     */
    static const uint32_t LEFT_BORDER = 48;
    static const uint32_t RIGHT_BORDER = 48;
    static const uint32_t SCREEN_WIDTH = 256;
    static const uint32_t DISPLAY_WIDTH = LEFT_BORDER + SCREEN_WIDTH + RIGHT_BORDER;
    static const uint32_t TOP_BORDER = 48;
    // The bottom border has 56 lines, out of which 48 lines are visible and 8 lines are hidden.
    static const uint32_t BOTTOM_BORDER = 56;
    static const uint32_t SCREEN_HEIGHT = 192;
    static const uint32_t DISPLAY_HEIGHT = TOP_BORDER + SCREEN_HEIGHT + BOTTOM_BORDER;
    static const uint32_t COLOUR_DEPTH = 4;

private:
    ZxView *m_pZxView;
    CBcmFrameBuffer *m_pFrameBuffer;
    uint32_t (*m_pScrTable)[256][256];  // screen pixel lookup table
    uint8_t *m_pVideoMem;               // Spectrum video memory
    uint32_t m_border;                   // Border colour index
    bool m_bDoubleBufferingEnabled;
    bool m_bVSync;
    bool m_bBufferSwapped;

    uint8_t *m_pBaseBuffer;
    uint8_t *m_pBuffer;
    uint8_t *m_pTargetBuffer8;
    uint32_t *m_pTargetBuffer32;

    // T-state at which the border was last changed (though port 0xFEu) on screen.
    uint32_t m_lastBorderChange;
    uint32_t m_firstBorderUpdate;
    uint32_t m_lastBorderUpdate;

    // Border colour cache. Each item in this array represents 8 pixels
    uint32_t m_borderColour[8] = { 0 };

    const uint16_t m_palette[16] = {
        0x0000u, // black
        0x0010u, // blue
        0x8000u, // red
        0x8010u, // magenta
        0x0400u, // green
        0x0410u, // cyan
        0x8400u, // yellow
        0x8410u, // white
        0x0000u, // black
        0x001Fu, // bright blue
        0xF800u, // bright red
        0xF81Fu, // bright magenta
        0x07E0u, // bright green
        0x07FFu, // bright cyan
        0xFFE0u, // bright yellow
        0xFFFFu // bright white
    };

public:
    [[maybe_unused]] const char *m_paletteColourName[16] = {
        "black",
        "blue",
        "red",
        "magenta",
        "green",
        "cyan",
        "yellow",
        "white",
        "black",
        "bright blue",
        "bright red",
        "bright magenta",
        "bright green",
        "bright cyan",
        "bright yellow",
        "bright white"
    };

};

#endif // ZXDISPLAY_H
