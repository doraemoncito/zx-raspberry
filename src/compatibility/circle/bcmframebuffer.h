/*
 * Copyright (c) 2020-2022 Jose Hernandez
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
#ifndef CIRCLE_BCMFRAMEBUFFER_H
#define CIRCLE_BCMFRAMEBUFFER_H


#include "types.h"
#include <QtGlobal>
#include <QtDebug>

class CBcmFrameBuffer {

private:
    unsigned nWidth = 0;
    unsigned nHeight = 0;
    unsigned nDepth = 0;
    unsigned nVirtualWidth = 0;
    unsigned nVirtualHeight = 0;

    uint8_t *buffer = nullptr;

public:

    uint32_t palette[256] = {0};


    CBcmFrameBuffer(unsigned nWidth, unsigned nHeight, unsigned nDepth, unsigned nVirtualWidth = 0, unsigned nVirtualHeight = 0) {

        this->nWidth = nWidth;
        this->nHeight = nHeight;
        this->nDepth = nDepth;
        this->nVirtualWidth = nVirtualWidth;
        this->nVirtualHeight = nVirtualHeight;

        // TODO: change data type according to the colour depth (min 8 bits)
        buffer = new uint8_t[nWidth * nHeight];
    };


    ~CBcmFrameBuffer(void) {};


    void SetPalette(u8 nIndex, u16 nRGB565) {

        if ((nIndex >= 0) && (nIndex <= 255)) {
            /* Z80 spectrum palette.  Please note that the bit order in attribute memory
             * is GRB in attribute whilst QImage is expecting RGB.  In this method, we are
             * re-ordering, i.e. swapping around the R and B components, to avoid having
             * to swap attribute bits around when decoding colors.
              *
             * - http://www.overtakenbyevents.com/lets-talk-about-the-zx-specrum-screen-layout/
             * - https://forum.arduino.cc/index.php?topic=285303.0
             *
             * The correct conversion, without colour component swapping is:
             *
             * uint32_t nRGBA;
             * nRGBA  = (uint32_t) (nRGB565 >> 11 & 0x1F) << (0+3);   // red
             * nRGBA |= (uint32_t) (nRGB565 >> 5  & 0x3F) << (8+2);   // green
             * nRGBA |= (uint32_t) (nRGB565       & 0x1F) << (16+3);  // blue
             * nRGBA |=                             0xFF  << 24;      // alpha
             */
            uint32_t nRGBA;
            nRGBA = (uint32_t) (nRGB565 & 0x1F) << (0 + 3);  // blue
            nRGBA |= (uint32_t) (nRGB565 >> 5 & 0x3F) << (8 + 2);  // green
            nRGBA |= (uint32_t) (nRGB565 >> 11 & 0x1F) << (16 + 3); // red
            nRGBA |= 0xFF << 24;     // alpha

            qDebug("Palette[0x%02X] = nRGBA: 0x%08X (RGB565: 0x%04X)", nIndex, nRGBA, nRGB565);

            palette[nIndex] = nRGBA;
        } else {
            qCritical("Palette index 0x%02X is out of bounds", nIndex);
        }
    };


    boolean Initialize() {

        return true;
    };


    u32 *GetBuffer() const {

        return reinterpret_cast<u32 *>(buffer);
    };


    u32 GetSize() const {

        return nWidth * nHeight;
    };

};


#endif // CIRCLE_BCMFRAMEBUFFER_H
