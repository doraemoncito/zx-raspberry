/*
 * Copyright (c) 2020-2024 Jose Hernandez
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
#ifndef ZXKEYBOARD_H
#define ZXKEYBOARD_H

class QKeyEvent;
class Z80emu;

// TODO: rename this class KeyboardMapper or KeyboardAdapter?
class ZxKeyboard {

public:
    ZxKeyboard();
    void reset();
    void keyPressEvent(Z80emu &zxRaspberry, QKeyEvent &event);
    void keyReleaseEvent(Z80emu &zxRaspberry, QKeyEvent &event);

private:

//    uint8_t m_keyboardPort[8] = {0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu };
//    enum PORT {
//        PORT_F7FE = 0,
//        PORT_FBFE,
//        PORT_FDFE,
//        PORT_FEFE,
//        PORT_EFFE,
//        PORT_DFFE,
//        PORT_BFFE,
//        PORT_7FFE
//    };

    uint8_t port_F7FE = 0xFFu;
    uint8_t port_FBFE = 0xFFu;
    uint8_t port_FDFE = 0xFFu;
    uint8_t port_FEFE = 0xFFu;
    uint8_t port_EFFE = 0xFFu;
    uint8_t port_DFFE = 0xFFu;
    uint8_t port_BFFE = 0xFFu;
    uint8_t port_7FFE = 0xFFu;

};


#endif //ZXKEYBOARD_H
