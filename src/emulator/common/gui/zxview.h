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
#ifndef ZX_VIEW_H
#define ZX_VIEW_H


#include <cstdint>
#include "zxrect.h"

class ZxView {

public:
    ZxView(ZxRect bounds);
    virtual ~ZxView();
    void printText(unsigned char *buffer, unsigned int column, unsigned int row, unsigned char ink, unsigned char paper,
                   char const *text) const;
    void clear(uint8_t *buffer, uint8_t paper);
    virtual void draw(uint8_t *buffer) = 0;

    ZxView *parent() const;
    void setParent(ZxView *parent);

    ZxRect const &bounds() const;

private:
    // TODO: move this to a singleton ZxCharset class;
    static uint8_t characters[0x90 * 0x08];
    const unsigned int charsetAddr = 0x3D00;

protected:
    ZxView *m_parent;
    ZxRect const m_bounds;

};


#endif //ZX_VIEW_H
