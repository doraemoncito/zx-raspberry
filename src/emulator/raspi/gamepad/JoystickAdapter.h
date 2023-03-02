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
#ifndef ZXRASPBERRY_JOYSTICKADAPTER_H
#define ZXRASPBERRY_JOYSTICKADAPTER_H

#include <circle/usb/usbgamepad.h>
#include <string>

class JoystickAdapter {

public:
    virtual std::string name() = 0;
    virtual void handleJoystickEvent(unsigned /* nDeviceIndex */, const TGamePadState & /*pState*/) {};

protected:
    bool isAxisButtonReleased(const TGamePadState &pState, TGamePadButton button, TGamePadAxis axis);
    bool isAxisButtonPressed(const TGamePadState &pState, TGamePadButton button, TGamePadAxis axis);
};

#endif //ZXRASPBERRY_JOYSTICKADAPTER_H
