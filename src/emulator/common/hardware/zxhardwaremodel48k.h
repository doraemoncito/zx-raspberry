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
#ifndef ZXRASPBERRY_ZXHARDWAREMODEL48K_H
#define ZXRASPBERRY_ZXHARDWAREMODEL48K_H


#include "zxhardwaremodel.h"

class ZxHardwareModel48k : public ZxHardwareModel {

public:
    ~ZxHardwareModel48k() override = default;

    std::string longModelName() override { return "ZX Spectrum 48K"; };
    std::string shortModelName() override { return "48k"; };
    uint32_t clockFrequency() override { return 3500000; };
    uint32_t tStatesPerScreenFrame() override { return 69888; };
    uint32_t tStatesPerScreenLine() override { return 224; };
    uint32_t upBorderHeight() override { return 64; };
    uint32_t tStatesToFirstScreenByte() override { return 14336; };
};


#endif //ZXRASPBERRY_ZXHARDWAREMODEL48K_H
