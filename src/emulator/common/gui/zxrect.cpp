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
#include <algorithm>
#include "zxpoint.h"
#include "zxrect.h"


ZxRect::ZxRect(int ax, int ay, int width, int height) {

    a = ZxPoint(ax, ay);
    b = ZxPoint(ax + width, ay + height);
}


ZxRect::ZxRect(ZxPoint p1, ZxPoint p2) {

    a = p1;
    b = p2;
}


ZxRect::ZxRect() = default;


void ZxRect::move(int aDX, int aDY) {

    a += ZxPoint(aDX, aDY);
    b += ZxPoint(aDX, aDY);
}


void ZxRect::grow(int aDX, int aDY) {

    a -= ZxPoint(aDX, aDY);
    b += ZxPoint(aDX, aDY);
}


void ZxRect::intersect(const ZxRect &r) {

    a = {std::max(this->a.getX(), r.a.getX()), std::max(this->a.getY(), r.a.getY())};
    b = {std::min(this->b.getX(), r.b.getX()), std::min(this->b.getY(), r.b.getY())};
}


void ZxRect::Union(const ZxRect &r) {

    a = {std::min(this->a.getX(), r.a.getX()), std::min(this->a.getY(), r.a.getY())};
    b = {std::max(this->b.getX(), r.b.getX()), std::max(this->b.getY(), r.b.getY())};
}


bool ZxRect::contains(const ZxPoint &p) const {

    return static_cast<bool>(
            p.getX() >= a.getX() &&
            p.getX() < b.getX() &&
            p.getY() >= a.getY() &&
            p.getY() < b.getY());
}


bool ZxRect::operator==(const ZxRect &r) const {

    return static_cast<bool>(a == r.a && b == r.b);
}


bool ZxRect::operator!=(const ZxRect &r) const {

    return static_cast<bool>(!(*this == r));
}


bool ZxRect::isEmpty() {

    return static_cast<bool>(a.getX() >= b.getX() || a.getY() >= b.getY());
}


int ZxRect::getAx() const {

    return a.getX();
}


int ZxRect::getAy() const {

    return a.getY();
}


int ZxRect::getBx() const {

    return b.getX();
}


int ZxRect::getBy() const {

    return b.getY();
}


int ZxRect::getWidth() const {

    return (b - a).getX();
}


int ZxRect::getHeight() const {

    return (b - a).getY();
}
