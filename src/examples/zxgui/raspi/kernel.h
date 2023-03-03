//
// kernel.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef KERNEL_H
#define KERNEL_H

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/types.h>
#include <zxdisplay.h>

enum TShutdownMode {
    ShutdownNone,
    ShutdownHalt,
    ShutdownReboot
};

class CBcmFrameBuffer;


class CKernel {
public:
    CKernel();
    ~CKernel();
    boolean Initialize();

    [[noreturn]] TShutdownMode Run();

private:
    static void KeyPressedHandler(const char *pString);
    static void ShutdownHandler();
    static void KeyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6]);
    static void KeyboardRemovedHandler(CDevice *pDevice, void *pContext);

    // do not change this order
    CActLED m_ActLED;
    CKernelOptions m_Options;
    CDeviceNameService m_DeviceNameService;
    CSerialDevice m_Serial;
    CExceptionHandler m_ExceptionHandler;
    CInterruptSystem m_Interrupt;
    CTimer m_Timer;
    CLogger m_Logger;
    CUSBHCIDevice m_USBHCI;

    CUSBKeyboardDevice *volatile m_pKeyboard;

    volatile TShutdownMode m_ShutdownMode;

    static CKernel *s_pThis;

    CBcmFrameBuffer *m_pFrameBuffer{};
    ZxDisplay m_zxDisplay;
    boolean m_showDialog;

    // Keyboard support
    unsigned char m_ucModifiers{};
    unsigned char m_rawKeys[6]{};

};

#endif // KERNEL_H
