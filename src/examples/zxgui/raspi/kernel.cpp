//
// kernel.cpp
//
// Spectrum screen emulator sample provided by Jose Luis Sanchez
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
#include <assert.h>
#include <circle/bcmframebuffer.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <circle/usb/usbkeyboard.h>
#include "kernel.h"
#include "common/ViajeAlCentroDeLaTierraScr.h"
#include "common/gui/zxdialog.h"
#include "common/gui/zxlabel.h"
#include "common/gui/zxrect.h"
#include "zxdisplay.h"

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = nullptr;


CKernel::CKernel() :
//    m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
    m_Timer (&m_Interrupt),
    m_Logger (m_Options.GetLogLevel (), &m_Timer),
    m_USBHCI (&m_Interrupt, &m_Timer, TRUE),		// TRUE: enable plug-and-play
    m_ShutdownMode(ShutdownNone),
    m_pKeyboard(nullptr),
    m_pAboutDialog(nullptr),
    m_showDialog(false) {

    s_pThis = this;
    m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel() {

    s_pThis = nullptr;
}

boolean CKernel::Initialize() {

	boolean bOK = m_Serial.Initialize (115200);

	if (bOK)
	{
		bOK = m_Logger.Initialize(&m_Serial);
	}

    if (bOK) {
        // Banner generated using this URL: http://patorjk.com/software/taag/#p=display&f=Standard&t=ZX%20GUI%0A
        m_Logger.Write(FromKernel, LogNotice, R"(  _______  __   ____ _   _ ___  )");
        m_Logger.Write(FromKernel, LogNotice, R"( |__  /\ \/ /  / ___| | | |_ _| )");
        m_Logger.Write(FromKernel, LogNotice, R"(   / /  \  /  | |  _| | | || |  )");
        m_Logger.Write(FromKernel, LogNotice, R"(  / /_  /  \  | |_| | |_| || |  )");
        m_Logger.Write(FromKernel, LogNotice, R"( /____|/_/\_\  \____|\___/|___| )");
        m_Logger.Write(FromKernel, LogNotice, R"(                                )");
        m_Logger.Write(FromKernel, LogNotice, " ");
        m_Logger.Write(FromKernel, LogNotice, "ZX Screen: a bare metal GUI test application");
        m_Logger.Write(FromKernel, LogNotice, "Copyright (c) 2020-2023 Jose Hernandez");
        m_Logger.Write(FromKernel, LogNotice, "Build date: " __DATE__ " " __TIME__);
    }

    if (bOK)
    {
        bOK = m_Interrupt.Initialize();
    }

    if (bOK)
    {
        bOK = m_Timer.Initialize();
    }

    if (bOK)
    {
        bOK = m_USBHCI.Initialize();
    }

	if (bOK)
	{
        /*
         * Our framebuffer uses a 4 bit palette to represent 16 colours that can be displayed on a ZX Spectrum at
         * anyone time.
         */
        m_pFrameBuffer = new CBcmFrameBuffer(
                ZxDisplay::SCREEN_WIDTH, ZxDisplay::SCREEN_HEIGHT, ZxDisplay::COLOUR_DEPTH,
                ZxDisplay::SCREEN_WIDTH, ZxDisplay::SCREEN_HEIGHT * 2, 0,
                true);
        bOK = m_zxDisplay.Initialize(ViajeAlCentroDeLaTierra_scr, m_pFrameBuffer);
    }

    m_Logger.Write(FromKernel, LogError, (bOK) ? "Initialisation completed successfully" : "Initialisation failed");

	return bOK;
}

void CKernel::handleAboutDialog() {
    if (m_showDialog && (m_pAboutDialog == nullptr)) {
        m_Logger.Write(FromKernel, LogNotice, "Creating the About dialog!");
        m_pAboutDialog = new ZxDialog(ZxRect(2, 12, 40, 10), "About ZX GUI");

        /*
         * The default printable characters (32 (space) to 127 (copyright)) are stored at the end of the Spectrum's ROM at
         * memory address 15616 (0x3D00) to 16383 (0x3FFF) and are referenced by the system variable CHARS which can be
         * found at memory address 23606/7. Interestingly, the value in CHARS is actually 256 bytes lower than the first
         * byte of the space character so that referencing a printable ASCII character does not need to consider the first
         * 32 characters. As such, the CHARS value (by default) holds the address 15360 (0x3C00).
         *
         * The UDG characters (Gr-A to Gr-U) are stored at the end of the Spectrum's RAM at memory address 65368 (0xFF58)
         * to 65535 (0xFFFF). As such, POKEing this address range has immediate effect on the UDG characters. The USR
         * keyword (when followed by a single quoted character) provides a quick method to reference these addresses from
         * BASIC. As with the printable characters, the location of the UDG characters is stored in the system variable UDG.
         *
         * Reference: https://enacademic.com/dic.nsf/enwiki/513468
         */
        m_pAboutDialog->insert(new ZxLabel(ZxRect(1, 2, 1, 1), "ZX GUI version 0.0.1"));
        m_pAboutDialog->insert(new ZxLabel(ZxRect(1, 3, 1, 1), "Copyright \x7F 2020-2023 Jose Hernandez"));
        m_pAboutDialog->insert(new ZxLabel(ZxRect(1, 6, 1, 1), "Build date: " __DATE__ " " __TIME__));
        m_zxDisplay.setUI(m_pAboutDialog);
    } else if (!m_showDialog && (m_pAboutDialog != nullptr)) {
        m_Logger.Write(FromKernel, LogNotice, "Destroying the About dialog!");
        delete m_pAboutDialog;
        m_pAboutDialog = nullptr;
        m_zxDisplay.setUI(m_pAboutDialog);
    }
}

[[noreturn]] TShutdownMode CKernel::Run() {

    m_Logger.Write (FromKernel, LogNotice, "Please attach a USB keyboard, if not already done!");

//    CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get()->GetDevice("ukbd1", FALSE);
////    CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice("ukbd1", FALSE);
//    if (pKeyboard == 0) {
//        m_Logger.Write(FromKernel, LogError, "Keyboard not found");
//    } else {
//        m_Logger.Write(FromKernel, LogNotice, "Keyboard found");
//        pKeyboard->RegisterKeyStatusHandlerRaw(KeyStatusHandlerRaw);
//    }

    bool toggleFrameBuffer = false;

    while (m_ShutdownMode == ShutdownNone) {

        // This must be called from TASK_LEVEL to update the tree of connected USB devices.
        boolean bUpdated = m_USBHCI.UpdatePlugAndPlay();

        if (bUpdated && m_pKeyboard == 0) {
            m_pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice("ukbd1", FALSE);
            if (m_pKeyboard != 0) {
                m_pKeyboard->RegisterRemovedHandler(KeyboardRemovedHandler);

#if 0    // set to 0 to test raw mode
                m_pKeyboard->RegisterShutdownHandler(ShutdownHandler);
                m_pKeyboard->RegisterKeyPressedHandler(KeyPressedHandler);
#else
                m_pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
#endif

                m_Logger.Write(FromKernel, LogNotice, "Just type something!");
            }
        }

        if (m_pKeyboard != 0) {
            // CUSBKeyboardDevice::UpdateLEDs() must not be called in interrupt context,
            // that's why this must be done here. This does nothing in raw mode.
            m_pKeyboard->UpdateLEDs();
        }

        handleAboutDialog();
        m_zxDisplay.update(FALSE);

        m_ActLED.Off();
		// The flash changes his state every 16 screen frames
		m_Timer.usDelay(319488);

        handleAboutDialog();
        m_zxDisplay.update(TRUE);

        m_ActLED.On();
		m_Timer.usDelay(319488);

//        m_pFrameBuffer->SetVirtualOffset(0, (toggleFrameBuffer = !toggleFrameBuffer) ? SCREEN_HEIGHT : 0);
    }

	return ShutdownHalt;
}


void CKernel::KeyPressedHandler (const char *pString)
{
    assert (s_pThis != nullptr);
#ifdef EXPAND_CHARACTERS
    while (*pString) {
        CString s;
        s.Format ("'%c' %d %02X\n", *pString, *pString, *pString);
        pString++;
        CLogger::Get()->Write (FromKernel, LogDebug, s);
        //s_pThis->m_Screen.Write (s, strlen (s));
    }
#else
    CLogger::Get()->Write (FromKernel, LogDebug, pString);
//    s_pThis->m_Screen.Write (pString, strlen (pString));
#endif
}

void CKernel::ShutdownHandler()
{
    assert (s_pThis != nullptr);
    s_pThis->m_ShutdownMode = ShutdownReboot;
}

void CKernel::KeyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6])
{
    assert (s_pThis != nullptr);

    // toggle the About box when the user presses F1
    if (RawKeys[0] == 0x3A) {
        s_pThis->m_showDialog = !s_pThis->m_showDialog;
    }

    CString Message;
    Message.Format ("Key status (modifiers %02X)", (unsigned) ucModifiers);
    bool showLogMessage = false;

    for (unsigned i = 0; i < 6; i++)
    {
        if (RawKeys[i] != 0)
        {
            showLogMessage = true;
            CString KeyCode;
            KeyCode.Format(" %02X", (unsigned) RawKeys[i]);

            Message.Append(KeyCode);
        }
    }

    if (showLogMessage) {
        s_pThis->m_Logger.Write(FromKernel, LogNotice, Message);
    }
}

void CKernel::KeyboardRemovedHandler (CDevice *pDevice, void *pContext)
{
    assert (s_pThis != nullptr);
    CLogger::Get()->Write (FromKernel, LogDebug, "Keyboard removed");
    s_pThis->m_pKeyboard = nullptr;
}
