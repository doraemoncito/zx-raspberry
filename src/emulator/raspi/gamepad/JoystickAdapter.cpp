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
#include "JoystickAdapter.h"

bool JoystickAdapter::isAxisButtonReleased(const TGamePadState &pState, TGamePadButton button, TGamePadAxis axis) {

    return !isAxisButtonPressed(pState, button, axis);
}

bool JoystickAdapter::isAxisButtonPressed(const TGamePadState &pState, TGamePadButton button, TGamePadAxis axis) {

//    if (axis < m_GamePadState.naxes) {
//        CString Value;
//        Value.Format("%03u", m_GamePadState.axes[axis].value);
////        PrintAt(nPosX, nPosY, Value, m_GamePadState.buttons & button ? TRUE : FALSE);
////    } else {
////        PrintAt(nPosX, nPosY, pName, m_GamePadState.buttons & button ? TRUE : FALSE);
//    }

    return (pState.buttons & button) != 0;
}
