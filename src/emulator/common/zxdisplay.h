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

    void setBorder(uint8_t m_border);
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

     * Follow these links for more information on the screen layout and timings:
     *
     * - <http://www.zxdesign.info/vidresearch.shtml>
     * - <http://www.zxdesign.info/vidparam.shtml>
     * - <http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout>
     */
    static const int LEFT_BORDER = 48;
    static const int RIGHT_BORDER = 48;
    static const int CANVAS_WIDTH = 256;
    static const int SCREEN_WIDTH = LEFT_BORDER + CANVAS_WIDTH + RIGHT_BORDER;
    static const int TOP_BORDER = 48;
    static const int BOTTOM_BORDER = 56;
    static const int CANVAS_HEIGHT = 192;
    static const int SCREEN_HEIGHT = TOP_BORDER + CANVAS_HEIGHT + BOTTOM_BORDER;
    static const int COLOUR_DEPTH = 4;

private:
    ZxView *m_pZxView;
    CBcmFrameBuffer *m_pFrameBuffer;
    uint32_t (*m_pScrTable)[256][256];  // lookup table
    uint8_t *m_pVideoMem;               // Spectrum video memory
    uint8_t m_border;                   // Border colour index
    bool m_bBorderChanged;
    bool m_bDoubleBufferingEnabled;
    bool m_bVSync;
    bool m_bBufferSwapped;

    uint8_t *m_pBaseBuffer;
    uint8_t *m_pBuffer;
    uint8_t *m_pTargetBuffer8;

    // T-state at which the border was last changed (though port 0xFEu) on screen.
    uint32_t m_lastBorderChanged;

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

};

#endif // ZXDISPLAY_H
