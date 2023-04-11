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
#ifndef ZXRASPBERRY_ZXHARDWAREMODEL_H
#define ZXRASPBERRY_ZXHARDWAREMODEL_H

#include <string>


class ZxHardwareModel {

public:
    virtual ~ZxHardwareModel() = default;

    virtual std::string longModelName() = 0;
    virtual std::string shortModelName() = 0;

    // Clock frequency in Hertz. i.e. clock cycles per second
    virtual uint32_t clockFrequency() = 0;

    // <http://www.zxdesign.info/vidresearch.shtml>
    virtual uint32_t tStatesPerScreenFrame() = 0;
    virtual uint32_t tStatesPerScreenLine() = 0;

    // Number of lines (both visible and invisible) in the top border
    virtual uint32_t upBorderHeight() = 0;

    // Number of T-states until the first screen byte is reached
    virtual uint32_t tStatesToFirstScreenByte() = 0;

    // Interrupt signal length in t-states
    virtual uint32_t lengthINT() = 0;

//protected:
//    CodeModel codeModel; // Código de modelo
//    String longModelName;   // Nombre largo del modelo de Spectrum
//    String shortModelName;   // Nombre corto del modelo de Spectrum
//    int clockFreq;
//    int tstatesFrame;    // t-states por cuadro de la imagen
//    int tstatesLine;     // t-states por línea de imagen
//    int upBorderWidth;   // Número de líneas del borde superior
//    int scanLines;       // Número de líneas de imagen
//    int firstScrByte;    // t-states hasta el primer byte de la pantalla
//    int lastScrUpdate;   // t-states última actualización de la pantalla
//    int outOffset;       //
//    int outBorderOffset; //
//    int lengthINT;       // Duración en t-states de la señal INT
//    boolean hasAY8912;  // Tiene un AY-3-8912?
//    boolean hasDisk;    // Tiene un controlador de disco y disquetera?

};


#endif //ZXRASPBERRY_ZXHARDWAREMODEL_H
