/*
 * Copyright (c) 2020-2023 Jose Hernandez
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

#include "zxspectrum.h"


void ZxSpectrum::run() {

    while (1) {
        generateFrame();
        drawFrame();
    }
}

void ZxSpectrum::generateFrame() {

//    /* Execute a frame's worth of T-states.  This will be roughly 20ms on a 48K ZX Spectrum.
//     * 20ms * 50 = 3.5MHz
//     */
//    z80emu->execute(spectrumModel->tStatesPerScreenFrame());
//
//    Clock::getInstance().endFrame();

}

void ZxSpectrum::drawFrame() {

}
