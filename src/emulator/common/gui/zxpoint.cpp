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
#include "zxpoint.h"


ZxPoint::ZxPoint() : x(0), y(0) {
}


ZxPoint::ZxPoint(int x, int y) : x(x), y(y) {
}


ZxPoint &ZxPoint::operator+=(const ZxPoint &adder) {

    x += adder.x;
    y += adder.y;
    return *this;
}


ZxPoint &ZxPoint::operator-=(const ZxPoint &subber) {

    x -= subber.x;
    y -= subber.y;
    return *this;
}


bool operator==(const ZxPoint &one, const ZxPoint &two) {

    return ((one.x == two.x) && (one.y == two.y));
}


bool operator!=(const ZxPoint &one, const ZxPoint &two) {

    return ((one.x != two.x) || (one.y != two.y));
}


ZxPoint operator-(const ZxPoint &one, const ZxPoint &two) {

    return {one.x - two.x, one.y - two.y};
}


ZxPoint operator+(const ZxPoint &one, const ZxPoint &two) {

    return {one.x + two.x, one.y + two.y};
}


int ZxPoint::getX() const {

    return x;
}


int ZxPoint::getY() const {

    return y;
}
