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
#include "BruceLeeScr.h"
#include "automania_sna.h"
#include "dingo_sna.h"
#include "../../../emulator/include/zx48k_rom.h"
#include "z80emu.h"
#include <circle/bcmframebuffer.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <assert.h>

static const char FromKernel[] = "kernel";

CKernel::CKernel(void) :
	m_Timer (&m_Interrupt),
	m_Logger (LogDebug, &m_Timer) {

	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel(void) {

	delete bcmFrameBuffer;
}

boolean CKernel::Initialize(void) {

	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}    

	if (bOK)
	{
		bOK = m_Logger.Initialize (&m_Serial);
	}

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
		bcmFrameBuffer = new CBcmFrameBuffer(352, 272, 4);
		z80emu = new Z80emu();
		bOK = m_SpectrumScreen.Initialize(z80emu->getRam() + 0x4000, bcmFrameBuffer);
	}

	return bOK;
}

TShutdownMode CKernel::Run(void) {

	// CONCEPTO DE PRUEBA: NO ESTAMOS OBSERVANDO LOS TEMPORIZADORES

	m_Logger.Write(FromKernel, LogNotice, "Concepto de prueba: emulador Sinclair ZX Spectrum - Edicion Raspberry Pi");
	m_Logger.Write(FromKernel, LogNotice, "Copyright (C) 2020 Jose Hernandez");
	m_Logger.Write(FromKernel, LogNotice, "Muchas gracias a Jose Luis Sanchez creador del ZXBaremulator por su ayuda y consejos");
	m_Logger.Write(FromKernel, LogNotice, "Fecha y hora de compilacion: " __DATE__ " " __TIME__);

	m_Logger.Write(FromKernel, LogNotice, "Cargando 48K ROM");
	z80emu->initialise(zx48k_rom, zx48k_rom_len);
	m_Logger.Write(FromKernel, LogNotice, "Cargando juego en formato snapshot");
	z80emu->loadSnapshot(automania_sna);
	// z80emu->loadSnapshot(dingo_sna);

	while (TRUE) {
#ifdef DEBUG
		m_Logger.Write (FromKernel, LogNotice, "Actualizando video");
#endif // DEBUG
		m_SpectrumScreen.Update(FALSE);
		m_ActLED.Off();
		m_Timer.usDelay(19968);
#ifdef DEBUG
		m_Logger.Write (FromKernel, LogNotice, "Ejecutando instruccions CPU");
#endif // DEBUG
		for (unsigned int i=0; i < 50000; i++) {
			z80emu->run();
		}
		m_ActLED.On();
	}

	return ShutdownHalt;
}
