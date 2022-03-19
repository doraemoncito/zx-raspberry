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
// Since C++ 11 this header provides all the size specific data types
#include <cstdint>
// This header inclusion provides the definition of size_t
#include <cstdio>
#include "util.h"


uint32_t bswap32(uint32_t ulValue) {
#if BYTE_SWAP_DISABLED
    return ulValue;
#else
    uint32_t word;
    word  = (uint32_t) ((ulValue >> 0x00) & 0xFF) << (24);  // low-low
    word |= (uint32_t) ((ulValue >> 0x08) & 0xFF) << (16);  // low-high
    word |= (uint32_t) ((ulValue >> 0x10) & 0xFF) << (8);   // high-low
    word |= (uint32_t) ((ulValue >> 0x18) & 0xFF) << (0);   // high-high
    return word;
#endif
};


uint16_t bswap16(uint16_t usValue) {
#if BYTE_SWAP_DISABLED
    return usValue;
#else
    uint16_t word;
    word  = (uint16_t) ((usValue >> 0x00) & 0xFF) << (8); // low
    word |= (uint16_t) ((usValue >> 0x08) & 0xFF) << (0); // high
    return word;
#endif
}


void *memset(void *pBuffer, int nValue, size_t nLength) {     
    for (unsigned int i=0; i<nLength; i++) {
        *(reinterpret_cast<uint8_t *>(pBuffer) + i) = (uint8_t) nValue;
    }
    return pBuffer; 
};
