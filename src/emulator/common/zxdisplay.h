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

class ZxDisplay {
public:
    ZxDisplay();
    ~ZxDisplay();

    bool Initialize(uint8_t *pVideoMem, CBcmFrameBuffer *pFrameBuffer);
    void update(bool flash);
    void setBorder(uint8_t m_border);

private:
    CBcmFrameBuffer *m_pFrameBuffer;
    uint32_t *m_pBuffer;                // Address of frame buffer
    uint32_t (*m_pScrTable)[256][256];  // lookup table
    uint8_t *m_pVideoMem;               // Spectrum video memory
    uint8_t m_border;                   // Border colour index
    bool m_borderChanged;
};

#endif // ZXDISPLAY_H
