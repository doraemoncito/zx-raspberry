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
#ifndef KERNEL_H
#define KERNEL_H

#include <memory>
#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/usb/usbgamepad.h>
#include <circle/types.h>
#include "zxdisplay.h"


class CBcmFrameBuffer;
class Z80emu;
class ZxHardwareModel;

enum TShutdownMode {
    ShutdownNone,
    ShutdownHalt,
    ShutdownReboot
};

class CKernel {
public:
    CKernel(void);
    ~CKernel(void);
    bool Initialize(void);
    TShutdownMode Run(void);

private:
    // do not change this order
    CMemorySystem m_Memory;
    CActLED m_ActLED;
    CKernelOptions m_Options;
    CDeviceNameService m_DeviceNameService;
    CSerialDevice m_Serial;
    CExceptionHandler m_ExceptionHandler;
    CInterruptSystem m_Interrupt;
    CTimer m_Timer;
    CLogger m_Logger;
    CUSBHCIDevice m_USBHCI;

    // Keyboard support
    unsigned char m_ucModifiers;
    unsigned char m_rawKeys[6];

    // Gamepad support
    CUSBGamePadDevice *m_pGamePad;
    TGamePadState m_GamePadState;
    int m_nPosX;
    int m_nPosY;

    ZxDisplay m_zxDisplay;
    CBcmFrameBuffer *bcmFrameBuffer;
    Z80emu *z80emu;

    TShutdownMode m_ShutdownMode = ShutdownNone;

    static CKernel *s_pThis;

    // TODO: move the keyboard handling routines to their own class
    static void KeyPressedHandler(const char *pString);

    static void ShutdownHandler(void);

    static void KeyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6]);

    // TODO: move the gamepad handling routines to their own class
    static void gamePadStatusHandler(unsigned nDeviceIndex, const TGamePadState *pState);

    ZxHardwareModel *spectrumModel;

    /* TODO: MOVE THESE CONSTANTS TO A SHARED MULTI-PLATFORM LOCATION.
     *
     * These are the visible screen dimensions, which are smaller than the actual dimensions suggested by the
     * screen timings since it takes the electron beam some time to fly back to the beginning or top of the screen:
     *
     * WIDTH:  48 pixel border + 256 pixel screen + 48 pixel border = 352 pixels = 44 characters
     * HEIGHT: 48 pixel border + 192 pixel screen + 56 pixel border = 296 pixels = 37 characters

     * Follow these links for more information on the screen layout and timings:
     *
     * - <http://www.zxdesign.info/vidresearch.shtml>
     * - <http://www.zxdesign.info/vidparam.shtml>
     * - <http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout>
     */
    const int LEFT_BORDER = 48;
    const int RIGHT_BORDER = 48;
    const int CANVAS_WIDTH = 256;
    const int SCREEN_WIDTH = LEFT_BORDER + CANVAS_WIDTH + RIGHT_BORDER;
    const int TOP_BORDER = 48;
    const int BOTTOM_BORDER = 56;
    const int CANVAS_HEIGHT = 192;
    const int SCREEN_HEIGHT = TOP_BORDER + CANVAS_HEIGHT + BOTTOM_BORDER;

    /* The Spectrum screen memory map is split into two sections:
     * - 6144 bytes (0x1800) worth of bitmap data, starting at memory address 0x4000
     * - 768 bytes (0x0300) of colour attribute data, immediately after the bitmap data at address 0x5800
     */
    int BITMAP_DATA_SIZE = 0x1800;      // 6144 bytes
    int ATTRIBUTE_DATA_SIZE = 0x0300;   // 768 bytes

    int tStatesToScreenPixel48k(int tstates);
    void buildScreenTables48k();

    // Tabla que contiene la direcci??n de pantalla del primer byte de cada
    // car??cter en la columna cero.
    uint32_t *scrAddr = new uint32_t[CANVAS_HEIGHT];

    // Tabla de traslaci??n entre t-states y la direcci??n de la pantalla del
    // Spectrum que se vuelca en ese t-state o -1 si no le corresponde ninguna.
    uint32_t *states2scr;
    // Tabla de traslaci??n de t-states al pixel correspondiente del borde.
    uint32_t *states2border;

    uint8_t *delayTstates;

    // T-state sequence we need to run to update the screen
    uint32_t *stepStates;
    uint32_t step;
    // Constante que indica que no hay un evento pr??ximo
    // El valor de la constante debe ser mayor que cualquier spectrumModel.tstatesframe
    uint32_t NO_EVENT = 0xFFFFFFFF;
    // t-states del pr??ximo evento
    uint32_t nextEvent = NO_EVENT;

    uint32_t firstBorderUpdate;
    uint32_t lastBorderUpdate;
    int borderMode;

};

#endif // KERNEL_H
