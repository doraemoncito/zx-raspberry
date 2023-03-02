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
#include "kernel.h"
#include "common/BruceLeeScr.h"
#include <circle/bcmframebuffer.h>
#include <circle/util.h>

static const char FromKernel[] = "kernel";

CKernel::CKernel() :
	m_Timer (&m_Interrupt),
	m_Logger (LogDebug, &m_Timer) {

	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel() = default;

boolean CKernel::Initialize() {

	boolean bOK = m_Serial.Initialize (115200);

	if (bOK)
	{
		bOK = m_Logger.Initialize (&m_Serial);
	}

    // Banner generated using this URL: http://patorjk.com/software/taag/#p=display&f=Standard&t=ZX%20Screen
    m_Logger.Write(FromKernel, LogNotice, R"(   _______  __  ____                           )");
    m_Logger.Write(FromKernel, LogNotice, R"( |__  /\ \/ / / ___|  ___ _ __ ___  ___ _ __  )");
    m_Logger.Write(FromKernel, LogNotice, R"(   / /  \  /  \___ \ / __| '__/ _ \/ _ \ '_ \ )");
    m_Logger.Write(FromKernel, LogNotice, R"(  / /_  /  \   ___) | (__| | |  __/  __/ | | |)");
    m_Logger.Write(FromKernel, LogNotice, R"( /____|/_/\_\ |____/ \___|_|  \___|\___|_| |_|)");
    m_Logger.Write(FromKernel, LogNotice, R"(                                              )");
    m_Logger.Write(FromKernel, LogNotice, " ");
    m_Logger.Write(FromKernel, LogNotice, "ZX Screen: a bare metal screen test application");
    m_Logger.Write(FromKernel, LogNotice, "Copyright (c) 2020-2023 Jose Hernandez");
    m_Logger.Write(FromKernel, LogNotice, "Build date: " __DATE__ " " __TIME__);

    if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
        m_pBcmFrameBuffer = new CBcmFrameBuffer(352, 272, 4);
        bOK = m_zxDisplay.Initialize(BruceLee_scr, m_pBcmFrameBuffer);
	}

	return bOK;
}

[[noreturn]] TShutdownMode CKernel::Run() {

	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	while (true) {
        m_zxDisplay.update(false);
		m_ActLED.Off();
		// The flash changes his state every 16 screen frames
		m_Timer.usDelay(319488);
        m_zxDisplay.update(true);
		m_ActLED.On();
		m_Timer.usDelay(319488);
	}

	return ShutdownHalt;
}
