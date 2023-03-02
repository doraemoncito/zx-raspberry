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
#include "CursorJoystickAdapter.h"
#include "keyboard.h"

std::string CursorJoystickAdapter::name() {

    return std::string("CURSOR/AGF/PROTEK");
}

void CursorJoystickAdapter::handleJoystickEvent(unsigned int nDeviceIndex, const TGamePadState &pState) {

    uint8_t port_F7FE = 0xFFu;
//    uint8_t port_FBFE = 0xFFu;
//    uint8_t port_FDFE = 0xFFu;
//    uint8_t port_FEFE = 0xFFu;
    uint8_t port_EFFE = 0xFFu;
//    uint8_t port_DFFE = 0xFFu;
    uint8_t port_BFFE = 0xFFu;
    uint8_t port_7FFE = 0xFFu;

    JoystickAdapter::handleJoystickEvent(nDeviceIndex, pState);

    // Cursor Joystick emulation
    if (isAxisButtonPressed(pState, GamePadButtonUp, GamePadAxisButtonUp)) {
        // key 7 (up)
        port_EFFE &= KEY_PRESSED_BIT3;
    }
    if (isAxisButtonPressed(pState, GamePadButtonLeft, GamePadAxisButtonLeft)) {
        // key 5 (left)
        port_F7FE &= KEY_PRESSED_BIT4;
    }
    if (isAxisButtonPressed(pState, GamePadButtonRight, GamePadAxisButtonRight)) {
        // key 8 (right)
        port_EFFE &= KEY_PRESSED_BIT2;
    }
    if (isAxisButtonPressed(pState, GamePadButtonDown, GamePadAxisButtonDown)) {
        // key 6 (down)
        port_EFFE &= KEY_PRESSED_BIT4;
    }

    if (isAxisButtonPressed(pState, GamePadButtonTriangle, GamePadAxisButtonTriangle)) {
        // TODO: map this button to a key
    }

    if (isAxisButtonPressed(pState, GamePadButtonSquare, GamePadAxisButtonSquare)) {
        // key BREAK SPACE
        port_7FFE &= KEY_PRESSED_BIT0;
    }

    if (isAxisButtonPressed(pState, GamePadButtonCircle, GamePadAxisButtonCircle)) {
        // key ENTER
        port_BFFE &= KEY_PRESSED_BIT0;
    }

    if (isAxisButtonPressed(pState, GamePadButtonCross, GamePadAxisButtonCross)) {
        // key 0 (cross)
        port_EFFE &= KEY_PRESSED_BIT0;
    }
}
