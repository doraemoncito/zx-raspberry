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
#ifndef ZXULA_H
#define ZXULA_H

#include <vector>
#include "gamepad/JoystickAdapter.h"
#include "gamepad/NoneGamePadAdapter.h"
#include "gamepad/KempstonGamePadAdapter.h"
#include "gamepad/FullerGamePadAdapter.h"
#include "gamepad/Sinclair1GamePadAdapter.h"
#include "gamepad/Sinclair2GamePadAdapter.h"
#include "gamepad/CursorJoystickAdapter.h"
#include "gamepad/KeyboardGamePadAdapter.h"

class Z80emu;
class CBcmFrameBuffer;

class ZxUla {

public:
    ZxUla(Z80emu &zxRaspberry, CBcmFrameBuffer &frameBuffer);

    void scanLineReset();
    void scanLineNext();

    void refreshInit();
    void refreshKeyboard(unsigned char ucModifiers, const unsigned char rawKeys[6]);
    void refreshGamepad(unsigned nDeviceIndex, const TGamePadState &pState);
    void refreshDone();

private:
    Z80emu &m_zxRaspberry;
    CBcmFrameBuffer &m_frameBuffer;

// TODO:
//    struct PortCache {
//        uint8_t port_011F = 0x00u;
//        uint8_t port_F7FE = 0xFFu;
//        uint8_t port_FBFE = 0xFFu;
//        uint8_t port_FDFE = 0xFFu;
//        uint8_t port_FEFE = 0xFFu;
//        uint8_t port_EFFE = 0xFFu;
//        uint8_t port_DFFE = 0xFFu;
//        uint8_t port_BFFE = 0xFFu;
//        uint8_t port_7FFE = 0xFFu;
//    } cache;

    uint8_t port_011F = 0x00u;
    uint8_t port_F7FE = 0xFFu;
    uint8_t port_FBFE = 0xFFu;
    uint8_t port_FDFE = 0xFFu;
    uint8_t port_FEFE = 0xFFu;
    uint8_t port_EFFE = 0xFFu;
    uint8_t port_DFFE = 0xFFu;
    uint8_t port_BFFE = 0xFFu;
    uint8_t port_7FFE = 0xFFu;

    void refreshPort(uint16_t port, uint8_t value);
    bool isAxisButtonPressed(const TGamePadState &pState, TGamePadButton Button, TGamePadAxis Axis);
    bool isAxisButtonReleased(const TGamePadState &pState, TGamePadButton Button, TGamePadAxis Axis);

    std::vector<JoystickAdapter *> m_gamepadAdapter {
        new NoneGamePadAdapter(),
        new KempstonGamePadAdapter(),
        new FullerGamePadAdapter(),
        new Sinclair1GamePadAdapter(),
        new Sinclair2GamePadAdapter(),
        new CursorJoystickAdapter(),
        new KeyboardGamePadAdapter()
    };

    uint32_t scanLineIndex = 0;

    uint8_t currentGamePadAdapter = 0;
    bool isSwitchingGamePadAdapter = false;

};


#endif //ZXULA_H
