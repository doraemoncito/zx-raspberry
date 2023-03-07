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
#include "zxlabel.h"


ZxLabel::ZxLabel(const ZxRect bounds, const char *text) : ZxView(bounds), text(text) {
}


void ZxLabel::draw(uint8_t *buffer, unsigned char ink = 0x0, unsigned char paper = 0xF) {

    clear(buffer, paper);
    printText(buffer, m_bounds.getAx(), m_bounds.getAy(), ink, paper, text);
}


void ZxLabel::draw(uint8_t *buffer) {

    printText(buffer, m_bounds.getAx(), m_bounds.getAy(), 0x0, 0xF, text);
}
