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
#ifndef ZXRASPBERRY_ZXGAMEPAD_H
#define ZXRASPBERRY_ZXGAMEPAD_H

#include <list>
#include <memory>
#include "gamepad/JoystickAdapter.h"
#include "gamepad/NullGamePadAdapter.h"
#include "gamepad/KempstonGamePadAdapter.h"
#include "gamepad/FullerGamePadAdapter.h"
#include "gamepad/Sinclair1GamePadAdapter.h"
#include "gamepad/Sinclair2GamePadAdapter.h"
#include "gamepad/CursorJoystickAdapter.h"
#include "gamepad/KeyboardGamePadAdapter.h"


class ZxGamepad : JoystickAdapter {

public:
    ZxGamepad() {

        adapters.push_back(std::move(std::make_unique<NullGamePadAdapter>()));
        adapters.push_back(std::move(std::make_unique<KempstonGamePadAdapter>()));
        adapters.push_back(std::move(std::make_unique<FullerGamePadAdapter>()));
        adapters.push_back(std::move(std::make_unique<Sinclair1GamePadAdapter>()));
        adapters.push_back(std::move(std::make_unique<Sinclair2GamePadAdapter>()));
        adapters.push_back(std::move(std::make_unique<CursorJoystickAdapter>()));
        adapters.push_back(std::move(std::make_unique<KeyboardGamePadAdapter>()));
    }

private:

    std::list<std::unique_ptr<JoystickAdapter>> adapters;
    unsigned int currentAdapter = 0;

};

#endif //ZXRASPBERRY_ZXGAMEPAD_H
