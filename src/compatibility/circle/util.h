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
#ifndef CIRCLE_UTIL_H
#define CIRCLE_UTIL_H


// Since C++ 11 this header provides all the size specific data types
#include <cstdint>

// This header inclusion provides the definition of size_t
#include <cstdio>


uint32_t bswap32(uint32_t ulValue);
uint16_t bswap16(uint16_t usValue);
void *memset(void *pBuffer, int nValue, size_t nLength);


#endif // CIRCLE_UTIL_H
