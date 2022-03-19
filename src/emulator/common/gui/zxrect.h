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
#ifndef ZX_RECT_H
#define ZX_RECT_H

#include "zxpoint.h"

class ZxRect {

public:
    ZxRect(int ax, int ay, int width, int height);
    ZxRect(ZxPoint p1, ZxPoint p2);
    ZxRect();
    void move(int aDX, int aDY);
    void grow(int aDX, int aDY);
    void intersect(const ZxRect &r);
    void Union(const ZxRect &r);
    bool contains(const ZxPoint &p) const;
    bool operator==(const ZxRect &r) const;
    bool operator!=(const ZxRect &r) const;
    bool isEmpty();
    int getAx() const;
    int getAy() const;
    int getBx() const;
    int getBy() const;
    int getWidth() const;
    int getHeight() const;

private:
    ZxPoint a, b;

};

#endif  // ZX_RECT_H
