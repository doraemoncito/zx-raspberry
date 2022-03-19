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
#ifndef ZX_POINT_H
#define ZX_POINT_H

class ZxPoint {

public:
    ZxPoint();
    ZxPoint(int x, int y);
    ZxPoint &operator+=(const ZxPoint &adder);
    ZxPoint &operator-=(const ZxPoint &subber);
    friend ZxPoint operator-(const ZxPoint &one, const ZxPoint &two);
    friend ZxPoint operator+(const ZxPoint &one, const ZxPoint &two);
    friend bool operator==(const ZxPoint &one, const ZxPoint &two);
    friend bool operator!=(const ZxPoint &one, const ZxPoint &two);
    int getX() const;
    int getY() const;

private:

    int x, y;

};

#endif // ZX_POINT_H
