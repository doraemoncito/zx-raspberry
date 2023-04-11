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
#include <cassert>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <memory>
#include <circle/bcmframebuffer.h>
#include <circle/cputhrottle.h>
#include <circle/gpiopin.h>
#include <circle/logger.h>
#include <circle/timer.h>
#include <circle/usb/usbkeyboard.h>
#include "common/clock.h"
#include "common/gui/zxlabel.h"
#include "common/gui/zxgroup.h"
#include "../include/zx48k_rom.h"
//#include "test_2scrn_y_ay8192_sna.h"
//#include "fpga48all_sna.h"
//#include "testkeys_sna.h"
//#include "overscan_sna.h"
//#include "automania_sna.h"
#include "aquaplane_sna.h"
//#include "shock_sna.h"
#include "kernel.h"
#include "Z80emu.h"
#include "zxula.h"
#include "common/hardware/zxhardwaremodel48k.h"

#define DEVICE_INDEX    1        // "upad1"

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = nullptr;


CKernel::CKernel() :
        m_Timer(&m_Interrupt),
        m_Logger(m_Options.GetLogLevel(), &m_Timer),
        m_USBHCI(&m_Interrupt, &m_Timer),
        m_ucModifiers(0),
        m_rawKeys({0, 0, 0, 0, 0, 0}),
        m_pGamePad(nullptr),
        m_ShutdownMode(ShutdownNone) {

    s_pThis = this;

    m_ActLED.Blink(5);    // show we are alive
}


CKernel::~CKernel() {

    delete m_pFrameBuffer;
    s_pThis = nullptr;
}


bool CKernel::Initialize() {

    bool bOK = m_Serial.Initialize(115200);

    if (bOK) {
        bOK = m_Logger.Initialize(&m_Serial);
    }

    // Banner generated using this URL: http://patorjk.com/software/taag/#p=display&f=Standard&t=ZX%20Raspberry%0A
    m_Logger.Write(FromKernel, LogNotice, R"(  _______  __  ____                 _                          )");
    m_Logger.Write(FromKernel, LogNotice, R"( |__  /\ \/ / |  _ \ __ _ ___ _ __ | |__   ___ _ __ _ __ _   _ )");
    m_Logger.Write(FromKernel, LogNotice, R"(   / /  \  /  | |_) / _` / __| '_ \| '_ \ / _ \ '__| '__| | | |)");
    m_Logger.Write(FromKernel, LogNotice, R"(  / /_  /  \  |  _ < (_| \__ \ |_) | |_) |  __/ |  | |  | |_| |)");
    m_Logger.Write(FromKernel, LogNotice, R"( /____|/_/\_\ |_| \_\__,_|___/ .__/|_.__/ \___|_|  |_|   \__, |)");
    m_Logger.Write(FromKernel, LogNotice, R"(                             |_|                         |___/ )");
    m_Logger.Write(FromKernel, LogNotice, " ");
    m_Logger.Write(FromKernel, LogNotice, "ZX Raspberry: a bare metal Sinclair ZX Spectrum emulator for Raspberry Pi");
    m_Logger.Write(FromKernel, LogNotice, "Copyright (c) 2020-2023 Jose Hernandez");
    m_Logger.Write(FromKernel, LogNotice, "Build date: " __DATE__ " " __TIME__);

    if (bOK) {
        bOK = m_Interrupt.Initialize();
        m_Logger.Write(FromKernel, LogNotice, "Initialising interrupt subsystem");
    }

    if (bOK) {
        m_Logger.Write(FromKernel, LogNotice, "Initialising timer subsystem");
        bOK = m_Timer.Initialize();
    }

    if (bOK) {
        m_Logger.Write(FromKernel, LogNotice, "Initialising USB subsystem");
        bOK = m_USBHCI.Initialize();
    }

    if (bOK) {
        m_Logger.Write(FromKernel, LogNotice, "Initialising display frame buffer");
        /*
         * Our framebuffer uses a 4 bit palette to represent 16 colours that can be displayed on a ZX Spectrum at
         * anyone time.
         */
        m_pFrameBuffer = new CBcmFrameBuffer(
                ZxDisplay::DISPLAY_WIDTH, ZxDisplay::DISPLAY_HEIGHT, ZxDisplay::COLOUR_DEPTH,
                ZxDisplay::DISPLAY_WIDTH, ZxDisplay::DISPLAY_HEIGHT * 2, 0,
                true);
        bOK = (m_pFrameBuffer != nullptr);
    }

    if (bOK) {
        m_pZxDisplay = new ZxDisplay();
        z80emu = new Z80emu(m_pZxDisplay);
        bOK = m_pZxDisplay->Initialize(z80emu->getRam() + 0x4000, m_pFrameBuffer);
    }

    return bOK;
}


unsigned clockTicksToMicroSeconds(unsigned ticks) {
//    float numerator = 1000000.0f;
//    float denominator = 700000000;
//    float us_per_tick = numerator / denominator;
//    float us_per_tick = 700;
    float us_per_tick = 1500;

    unsigned us = ticks / us_per_tick;

    return us;
}


[[noreturn]] TShutdownMode CKernel::Run() {

    /*
     * Configure push button 3 on the Maker pHAT board to work in pull up input model as described in the
     * [Maker pHAT reference manual](https://docs.google.com/document/d/1iDYLyoQKOPJRD5m5ivu8Ivv6zyeLIc3IxUtRKOx7n8k/view)
     * which indicates SW3 is connected to GPIO 20.
     */
    CGPIOPin m_ResetPin(20, TGPIOMode::GPIOModeInputPullUp);
    m_Logger.Write(FromKernel, LogNotice, "Reboot button enabled: press SW3 (GPIO 20) on Maker pHAT to reboot");

    spectrumModel = new ZxHardwareModel48k();
    Clock::getInstance().setSpectrumModel(spectrumModel);

    // FIXME: the next few variables belong in the specific hardware class
    stepStates = new uint32_t[BITMAP_DATA_SIZE];
    states2scr = new uint32_t[spectrumModel->tStatesPerScreenFrame() + 100];
    states2border = new uint32_t[spectrumModel->tStatesPerScreenFrame() + 100];
    delayTstates = new uint8_t[spectrumModel->tStatesPerScreenFrame() + 200];

//    buildScreenTables48k();

//  CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get()->GetDevice("ukbd1", FALSE);
    CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice("ukbd1", FALSE);
    if (pKeyboard == 0) {
        m_Logger.Write(FromKernel, LogError, "Keyboard not found");
    } else {
        m_Logger.Write(FromKernel, LogNotice, "Keyboard found");
    }

#if 0    // set to 0 to test raw mode
    pKeyboard->RegisterShutdownHandler(ShutdownHandler);
    pKeyboard->RegisterKeyPressedHandler(KeyPressedHandler);
#else
    pKeyboard->RegisterKeyStatusHandlerRaw(KeyStatusHandlerRaw);
#endif

    // get pointer to gamepad device and check if it is supported
    m_pGamePad = (CUSBGamePadDevice *) m_DeviceNameService.GetDevice("upad", DEVICE_INDEX, FALSE);
    if (m_pGamePad == 0) {
        m_Logger.Write(FromKernel, LogWarning, "Gamepad not found");
    } else {
        m_Logger.Write(FromKernel, LogNotice, "Gamepad found");

        if (!(m_pGamePad->GetProperties() & GamePadPropertyIsKnown)) {
            m_Logger.Write(FromKernel, LogError, "Gamepad mapping is not known");
        }

        // get initial state from gamepad and register status handler
        const TGamePadState *pState = m_pGamePad->GetInitialState();
        assert (pState != nullptr);
        gamePadStatusHandler(DEVICE_INDEX - 1, pState);

        m_pGamePad->RegisterStatusHandler(gamePadStatusHandler);

        // Briefly rumble the gamepad if it is supported
        if (m_pGamePad->GetProperties() & GamePadPropertyHasRumble) {
            if (m_pGamePad->SetRumbleMode(GamePadRumbleModeLow)) {
                m_Timer.MsDelay(250);
                m_pGamePad->SetRumbleMode(GamePadRumbleModeOff);
            }
        }
    }

    m_Logger.Write(FromKernel, LogNotice, "Loading 48K ROM");
    z80emu->initialise(zx48k_rom, zx48k_rom_len);

    m_Logger.Write(FromKernel, LogNotice, "Loading game in SNA format");
//    z80emu->loadSnapshot(shock_sna);
    z80emu->loadSnapshot(aquaplane_sna);
//    z80emu->loadSnapshot(overscan_sna);
//    z80emu->loadSnapshot(test_2scrn_y_ay8192_sna);
//    z80emu->loadSnapshot(automania_sna);
//    z80emu->loadSnapshot(testkeys_sna);
//    z80emu->loadSnapshot(fpga48all_sna);

    CCPUThrottle *ccpuThrottle = new CCPUThrottle(CPUSpeedUnknown);
//    CCPUThrottle *ccpuThrottle = CCPUThrottle::Get();
    m_Logger.Write(FromKernel, LogNotice, "ZX Spectrum 48K frame rate: 50.08Hz");
    unsigned clockRate = ccpuThrottle->GetClockRate();
    m_Logger.Write(FromKernel, LogNotice, "Host CPU clock rate is %uHz", clockRate);
    const unsigned clockTicksPerFrame = clockRate / 50;  // Raspberry Pi clock ticks per frame
    m_Logger.Write(FromKernel, LogNotice, "Host CPU clock ticks per ZX Spectrum frame: %u", clockTicksPerFrame);
    m_Logger.Write(FromKernel, LogNotice, "Host CPU clock ticks per microsecond: %u", clockRate / 1000000);

    bool flash = false;
    unsigned int frameCounter = 0;
    step = 0;

    std::unique_ptr<ZxUla> zxUla(new ZxUla(*z80emu, *m_pFrameBuffer));

    uint32_t nextEvent = stepStates[0];

#ifdef DEBUG
    m_Logger.Write(FromKernel, LogNotice, "T-states per frame: %u", spectrumModel->tStatesPerScreenFrame());
#endif // DEBUG

    while (ShutdownNone == m_ShutdownMode) {
//#ifdef DEBUG
//        m_Logger.Write(FromKernel, LogNotice, "Running CPU instructions");
//#endif // DEBUG

        /* Flash the Raspberry Pi LED on and off following the ZX Spectrum attribute flash cycle. This will give a
         * visual cue that the emulator is actually running.
         */
        (flash) ? m_ActLED.On() : m_ActLED.Off();

        unsigned startClockTicks = m_Timer.GetClockTicks();

        /* https://stackoverflow.com/questions/112439/cpu-emulation-and-locking-to-a-specific-clock-speed
         *
         * In the Spectrum lexicon, a T-state is just a "time state" — a single cycle of the clock running at 3.5 Mhz
         * (3500000 instructions per-second).
         *
         * There are 69888 T-states per frame in a 48K machine.
         *
         * According to the 128K ZX Spectrum Technical Information, there are 70908 T states per frame, and the '50 Hz'
         * interrupt occurs at 50.01 Hz.
         * 70908 t-states per frame * 50.01 Hz = 3.546 million t-states per second. Which is the clock rate of the
         * processor (slightly rounded, because the 50.01 is slightly rounded).
         *
         * A single line of output takes 224 t-states on 48K ZX Spectrum, but 228 t-states on 128K ZX Spectrum models.
         * 48K ZX Spectrum has Z80 clocked at precisely 3.5MHz (see faqwiki.zxnet.co.uk/wiki/ZX_Spectrum_16K/48K), so
         * it has line frequency 3500000/224=15625Hz and, with 312 lines per frame, frame frequency 50.08Hz.
         * 128K ZX Spectrum models have Z80 clocked at 3.5469MHz (see faqwiki.zxnet.co.uk/wiki/ZX_Spectrum_128),
         * so its line and frame frequencies are 3546900/228~15556.6Hz and, with 311 lines per frame, 50.02Hz.
         *
         * https://retrocomputing.stackexchange.com/questions/12832/zx-spectrum-what-is-the-t-state-value-with-reference-to-sound
         *
         * https://fms.komkon.org/EMUL8/HOWTO.html#LABHB
         * Execute a frame worth of T-States (i.e. ZX spectrum clock cycles)
         *
         * 128K ZX Spectrum has 70908 T states per frame of which 228 will be used by the ULA to draw the screen.
         * In addition to this, the target T states we also need to add deficit or remove any excess T states from the previous frame.
         * int64_t numberOfStatesRemainingInFrame = 70908 - 228 - z80emu->getStates();
         */

        /* Refresh keyboard and gamepad IO.  A more accurate version of the emulator may want to check the state of
         * the keyboard and gamepad when the IO ports are read by the ULA.
         */
        zxUla->refreshInit();
        zxUla->refreshKeyboard(m_ucModifiers, m_rawKeys);
        zxUla->refreshGamepad(DEVICE_INDEX + 1, m_GamePadState);
        zxUla->refreshDone();

//        zxUla->scanLineReset();

//        while (step < BITMAP_DATA_SIZE) {
//            z80emu->run(stepStates[step]);
//            if (Clock::getInstance().getTstates() >= nextEvent) {
//                nextEvent = step < stepStates.length ? stepStates[step] : NO_EVENT;
////                updateBorder(Clock::getInstance().getTstates());
//            }
//        }

        /* Execute a frame's worth of T-states.  This will be roughly 20ms on a 48K ZX Spectrum.
         * 20ms * 50 = 3.5MHz
         */
        z80emu->execute(spectrumModel->tStatesPerScreenFrame());

        Clock::getInstance().endFrame();
//        step = 0;
//        nextEvent = stepStates[0];

        /* A single ZX Spectrum display row takes 224 T-States, including the horizontal fly-back. For every T-State,
         * 2 pixels are written to the display, so 128 T-States will pass for the 256 pixels in a display row. The ZX
         * Spectrum is clocked at 3.5 MHz, so if 2 pixels are written in a single CPU clock cycle, the pixel clock of
         * our display must be 7 MHz. A single line thus takes 448 pixel clock cycles.
         * http://www.zxdesign.info/vidparam.shtml
         */

        // The flash changes its state every 16 screen frames
        if (++frameCounter % 16 == 0) {
            flash = !flash;
        }

//#ifdef DEBUG
//        m_Logger.Write(FromKernel, LogNotice, "Refreshing video framebuffer");
//#endif // DEBUG

        if (pKeyboard == nullptr && m_pZxDisplay->getUI() == nullptr) {
            auto zxMessage = new ZxGroup({2, 30, 38, 2});
            auto zxLabel1 = new ZxLabel({0, 0, 36, 1}, "Keyboard not found!");
            auto zxLabel2 = new ZxLabel({0, 1, 36, 1}, "Please connect keyboard to continue.");
            zxMessage->insert(zxLabel1);
            zxMessage->insert(zxLabel2);
            m_pZxDisplay->setUI(zxMessage);
        } else if (pKeyboard != nullptr && m_pZxDisplay->getUI() != nullptr) {
            delete m_pZxDisplay->getUI();
            m_pZxDisplay->setUI(nullptr);
        }

        m_pZxDisplay->update(flash);

        unsigned endClockTicks = m_Timer.GetClockTicks();
        unsigned usDelay = clockTicksToMicroSeconds(clockTicksPerFrame - (endClockTicks - startClockTicks));
//        m_Logger.Write (FromKernel, LogNotice, "Delay (microseconds): %u", usDelay);
        m_Timer.usDelay(usDelay);

        // Check whether GPIO pin 20, SW3 on the Maker pHAT, has been pressed and reboot the Raspberry Pi if so.
        if (m_ResetPin.Read() == 0) {
            m_Logger.Write(FromKernel, LogNotice, "Reset button (SW3) on GPIO pin 20 pressed; rebooting device!");
            m_ShutdownMode = ShutdownReboot;
            // Apply a 100-millisecond delay to allow the message to be sent over the serial port before rebooting.
            m_Timer.usDelay(100000);
            break;
        }
    }

    return m_ShutdownMode;
}

// TODO: move the keyboard support code to its own class

void CKernel::KeyPressedHandler(const char *pString) {

    assert (s_pThis != nullptr);
    s_pThis->m_Logger.Write(FromKernel, LogError, "%c", *pString);
//    s_pThis->m_Screen.Write (pString, strlen (pString));
}


void CKernel::ShutdownHandler() {

    assert (s_pThis != nullptr);
    s_pThis->m_ShutdownMode = ShutdownReboot;
}


void CKernel::KeyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6]) {

    assert (s_pThis != nullptr);
    s_pThis->m_ucModifiers = ucModifiers;
    memcpy(&s_pThis->m_rawKeys, RawKeys, 6 * sizeof(unsigned char));
}


void CKernel::gamePadStatusHandler(unsigned nDeviceIndex, const TGamePadState *pState) {

    // At present, we can only handle one gamepad
    if (nDeviceIndex != DEVICE_INDEX - 1) {
        return;
    }

    assert (s_pThis != nullptr);
    assert (pState != nullptr);
    memcpy(&s_pThis->m_GamePadState, pState, sizeof *pState);
}
