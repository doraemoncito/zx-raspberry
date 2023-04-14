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
#include <streambuf>
#include <istream>
#include <cstring>
#include <circle/logger.h>
#include <circle/util.h>
#include <common/hardware/zxhardwaremodel48k.h>
#include "zxdisplay.h"
#include "Z80emu.h"
#include "keyboard.h"
#include "clock.h"


struct membuf: std::streambuf {
    membuf(unsigned char const* base, size_t size) {
        char* p(const_cast<char*>(reinterpret_cast<const char *>(base)));
        this->setg(p, p, p + size);
    }
};

struct imemstream: virtual membuf, std::istream {
    imemstream(unsigned char const* base, size_t size)
        : membuf(base, size),
        std::istream(static_cast<std::streambuf*>(this)) {
    }
};

using namespace std;
static ZxHardwareModel48k model48K;


static const char msgFromULA[] = "[ULA    ]";


Z80emu::Z80emu(ZxDisplay *pZxDisplay) :
    cpu(this),
    m_border(0x07u),
    m_pZxDisplay(pZxDisplay)
{
    m_pZ80Ram = reinterpret_cast<uint8_t *>(new uint32_t[0x4000]);
    m_pZ80Port = reinterpret_cast<uint8_t *>(new uint32_t[0x4000]);
//    m_pZ80Ram = new uint8_t[0x10000];
//    m_pZ80Port = new uint8_t[0x10000];

    Clock::getInstance().reset();

    // FIXME: Implement keyboard support. For now, we'll turn the keyboard off until we implement it.
    // Bits are set to 0 for any key that is pressed and 1 for any key that is not pressed. Multiple key presses can be read simultaneously.
    // http://www.breakintoprogram.co.uk/computers/zx-spectrum/keyboard
    m_pZ80Port[0xFEFE] = 0xFF;
    m_pZ80Port[0xFDFE] = 0xFF;
    m_pZ80Port[0xFBFE] = 0xFF;
    m_pZ80Port[0xF7FE] = 0xFF;
    m_pZ80Port[0xEFFE] = 0xFF;
    m_pZ80Port[0xDFFE] = 0xFF;
    m_pZ80Port[0xBFFE] = 0xFF;
    m_pZ80Port[0x7FFE] = 0xFF;
    m_pZ80Port[0x00FE] = 0xFF;

    m_contendedRamPage[0] = m_contendedIOPage[0] = false;
    m_contendedRamPage[1] = m_contendedIOPage[1] = true;
    m_contendedRamPage[2] = m_contendedIOPage[2] = false;
    m_contendedRamPage[3] = m_contendedIOPage[3] = false;

    m_pDelayTstates = new ::uint8_t[model48K.tStatesPerScreenFrame() + 256];
    //memset(m_pDelayTstates, (byte) 0x00);

    for (int idx = 14335; idx < 57247; idx += model48K.tStatesPerScreenLine()) {
        for (int ndx = 0; ndx < 128; ndx += 8) {
            int frame = idx + ndx;
            m_pDelayTstates[frame++] = 6;
            m_pDelayTstates[frame++] = 5;
            m_pDelayTstates[frame++] = 4;
            m_pDelayTstates[frame++] = 3;
            m_pDelayTstates[frame++] = 2;
            m_pDelayTstates[frame++] = 1;
            m_pDelayTstates[frame++] = 0;
            m_pDelayTstates[frame++] = 0;
        }
    }

}

Z80emu::~Z80emu() = default;

uint8_t Z80emu::fetchOpcode(uint16_t address) {
    // 3 clocks to fetch opcode from RAM and 1 execution clock = 4 t-states
    Clock::getInstance().addTstates(4);
    return m_pZ80Ram[address];
}

uint8_t Z80emu::peek8(uint16_t address) {
    // 3 clocks for read byte from RAM
    Clock::getInstance().addTstates(3);
    return m_pZ80Ram[address];
}

void Z80emu::poke8(uint16_t address, uint8_t value) {
    // 3 clocks for write byte to RAM
    Clock::getInstance().addTstates(3);
    m_pZ80Ram[address] = value;
}

uint16_t Z80emu::peek16(uint16_t address) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // Order matters, first read lsb, then read msb, don't "optimize"
    uint8_t lsb = peek8(address);
    uint8_t msb = peek8(address + 1);
    return (msb << 0x08u) | lsb;
#else
    // 6 clocks to read word from RAM
    Clock::getInstance().addTstates(6);
    /* It is safe to read 16 bit words without explicit byte swapping if the
     * byte order of the host machine matches that of the Zilog Z80 CPU we are
     * emulating, in other words, little endian.  The 16 read and 16 bit write
     * will cancel each other out.
     */
    return *reinterpret_cast<uint16_t *>(&m_pZ80Ram[address]);
#endif
}

void Z80emu::poke16(uint16_t address, RegisterPair word) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // Order matters, first write lsb, then write msb, don't "optimize"
    poke8(address, word.byte8.lo);
    poke8(address + 1, word.byte8.hi);
#else
    // 6 clocks to write word to RAM
    Clock::getInstance().addTstates(6);
    /* It is safe to write 16 bit words without explicit byte swapping if the
     * byte order of the host machine matches that of the Zilog Z80 CPU we are
     * emulating, in other words, little endian.  The 16 read and 16 bit write
     * will cancel each other out.
     */
    *reinterpret_cast<uint16_t *>(&m_pZ80Ram[address]) = word.word;
#endif
}

uint8_t Z80emu::inPort(uint16_t port) {
    // 4 clocks for read byte from bus
    Clock::getInstance().addTstates(4);

    /* If port 0xXXFE is read from (where XX can be any hexadecimal number), the highest eight address lines are used
     * to select, via a zero on one of these lines, a particular half-row of five keys.
     *
     * A zero in one of the five lowest bits of the port return value means that the corresponding key is pressed.
     * If more than one address line is made low, the result is the logical AND of all single inputs, so a zero in a
     * bit means that at least one of the appropriate keys is pressed.
     *
     * https://neuro.me.uk/projects/wos/sinclairfaq.dev/ng/cssfaq/reference/48kreference.htm
     */
    // TODO: extract this code to a keyboard ULA class
    if ((port & 0x00FFu) == 0x00FEu) {  // ZX spectrum ULA allocated port
        uint8_t value = 0xFF;

        const uint8_t addressLine = (port >> 0x08u) & 0xFFu;
        if ((addressLine & (0xFFu - 0xF7u)) == 0x00u) {
            // 1 2 3 4 5
            value &= m_pZ80Port[0xF7FEu];
        }
        if ((addressLine & (0xFFu - 0xFBu)) == 0x00u) {
            // Q W E R T
            value &= m_pZ80Port[0xFBFEu];
        }
        if ((addressLine & (0xFFu - 0xFDu)) == 0x00u) {
            // A S D F G
            value &= m_pZ80Port[0xFDFEu];
        }
        if ((addressLine & (0xFFu - 0xFEu)) == 0x00u) {
            // SHIFT Z X C V
            value &= m_pZ80Port[0xFEFEu];
        };
        if ((addressLine & (0xFFu - 0xEFu)) == 0x00u) {
            // 0 9 8 7 6
            value &= m_pZ80Port[0xEFFEu];
        }
        if ((addressLine & (0xFFu - 0xDFu)) == 0x00u) {
            // P O I U Y
            value &= m_pZ80Port[0xDFFEu];
        }
        if ((addressLine & (0xFFu - 0xBFu)) == 0x00u) {
            // ENTER L K J H
            value &= m_pZ80Port[0xBFFEu];
        }
        if ((addressLine & (0xFFu - 0x7Fu)) == 0x00u) {
            // SPACE-BREAK SYMBOL-SHIFT M N B
            value &= m_pZ80Port[0x7FFEu];
        };

        return value;
    } else if (port == 0x011Fu) {  // Kempston joystick port
        // No need to do anything here at this point
    } else {
        // Log the port number that the emulated ZX Spectrum software is trying to access
        CLogger::Get()->Write(msgFromULA, LogDebug, "[PORT IN ] port 0x%04X <-- value 0x%02X", port, m_pZ80Port[port]);
    }

    return m_pZ80Port[port];
}

/*
 * Internal emulator call (not a call made by the emulated software) to the processor port `out` function.
 */
// FIXME: Rename this to make clear that we are setting the value of the port as if we were reading from an external
// device, i.e. the keyboard or joystick.  "setPort"or "internalSetPort" might be a better name for this method.
void Z80emu::internalOutPort(uint16_t port, uint8_t value) {
    if (m_pZ80Port[port] != value) {
#ifdef DEBUG
        CLogger::Get()->Write(msgFromULA, LogDebug, "[PORT INT] value 0x%02X --> port 0x%04X", value, port);
#endif //DEBUG
        m_pZ80Port[port] = value;
    }
}


/*
 * Las operaciones de I/O se producen entre los ciclos T3 y T4 de la CPU,
 * y justo ahí es donde podemos encontrar la contención en los accesos. Los
 * ciclos de contención son exactamente iguales a los de la memoria, con los
 * siguientes condicionantes dependiendo del estado del bit A0 y de si el
 * puerto accedido se encuentra entre las direcciones 0x4000-0x7FFF:
 *
 * High byte in 0x40 (0xc0) to 0x7f (0xff)?     Low bit  Contention pattern
 *                                      No      Reset    N:1, C:3
 *                                      No      Set      N:1, N:3
 *                                      Yes     Reset    C:1, C:3
 *                                      Yes     Set      C:1, C:1, C:1, C:1
 *
 * La columna 'Contention Pattern' se lee 'N:x', no contención x ciclos
 * 'C:n' se lee contención seguido de n ciclos sin contención.
 * Así pues se necesitan dos rutinas, la que añade el t-estado inicial
 * con sus contenciones cuando procede y la que añade los 3 estados finales
 * con la contención correspondiente.
 */
void Z80emu::preIO(int port) {

    // If this is a contented IO page
    if (m_contendedIOPage[port >> 14]) {
        int delayTstates = m_pDelayTstates[Clock::getInstance().getTstates()] + 1;
        CLogger::Get()->Write(msgFromULA, LogDebug, "(preIO) Adding %d delay T-states");
        Clock::getInstance().addTstates(delayTstates);
//        if (Clock::getInstance().getTstates() >= nextEvent) {
//            updateScreen(Clock::getInstance().getTstates());
//        }
    } else {
        Clock::getInstance().addTstates(1);
    }
}


void Z80emu::outPort(uint16_t port, uint8_t value) {
    preIO(port);

    // 4 clocks for write byte to bus
    Clock::getInstance().addTstates(4);

    /* NOTE: All even ports on the ZX spectrum are allocated to the ULA but to avoid problems with other I/O devices
     * only Port 0xFE should be used.  We do however need to check since many programs will write to any even port to
     * do things like change the border, etc...
     */
    if ((port & 0x0001) == 0) {
        /* OUT to port xxFE (the high byte is ignored) will set the border colour using the lowest three bits
         * {d2, d1, d0}, drive the MIC socket with d3 and the EAR socket (loudspeaker) with d4. d5 to d7 are not used.
         * The EAR and MIC sockets are connected only by resistors, so activating one activates the other; the EAR is
         * generally used for output as it produces a louder sound.
         *
         * Reference:
         *  - [Sinclair Wiki: ZX Spectrum ULA](https://faqwiki.zxnet.co.uk/wiki/ZX_Spectrum_ULA)
         *  - [World of Spectrum: 16K / 48K ZX Spectrum Reference](https://worldofspectrum.org/faq/reference/48kreference.htm#PortFE)
         *
         * Bit   7   6   5   4   3   2   1   0
         *     +-------------------------------+
         *     |   |   |   | E | M |   Border  |
         *     +-------------------------------+
         */

        // Update the border but only if the border colour has actually changed.
        uint8_t maskedValue = value & 0x07u;
        if (m_border != maskedValue) {
            auto tstates = Clock::getInstance().getTstates();
            m_border = maskedValue;
//#ifdef DEBUG
            CLogger::Get()->Write(msgFromULA, LogDebug,
                                  "(OutPort) Frame: %5d; T-states: %5d; Port: 0x%04X; Value: %d (%s)",
                                  Clock::getInstance().getFrames(),
                                  tstates, port, maskedValue, m_pZxDisplay->m_borderColourName[maskedValue]);
//#endif //DEBUG
            m_pZxDisplay->updateBorder(m_border, tstates);
        }

    }

    m_pZ80Port[port] = value;
}

void Z80emu::addressOnBus(uint16_t /* address */, int32_t wstates) {
    // Additional clocks to be added on some instructions
    Clock::getInstance().addTstates(wstates);
}

void Z80emu::interruptHandlingTime(int32_t wstates) {
    Clock::getInstance().addTstates(wstates);
}

bool Z80emu::isActiveINT() {
    int64_t tmp = Clock::getInstance().getTstates();

    if (tmp >= model48K.tStatesPerScreenFrame())
        tmp -= static_cast<int64_t>(model48K.tStatesPerScreenFrame());

    return ((tmp >= 0) && (tmp < model48K.lengthINT()));
}

#ifdef WITH_EXEC_DONE
void Z80emu::execDone(void) {}
#endif

#ifdef WITH_BREAKPOINT_SUPPORT
/* Callback for notify at PC address */
uint8_t Z80emu::breakpoint(uint16_t /* address */, uint8_t opcode) {
    // Emulate CP/M Syscall at address 5
    switch (cpu.getRegC()) {
        case 0: // BDOS 0 System Reset
        {
//            cout << "Z80 reset after " << m_tstates << " t-states" << endl;
            cout << "Z80 reset after " << Clock::getInstance().getTstates() << " t-states" << endl;
            finish = true;
            break;
        }
        case 2: // BDOS 2 console char output
        {
            cout << (char) cpu.getRegE();
            break;
        }
        case 9: // BDOS 9 console string output (string terminated by "$")
        {
            uint16_t strAddr = cpu.getRegDE();
            uint16_t endAddr = cpu.getRegDE();
            while (m_pZ80Ram[endAddr++] != '$');
            std::string message((const char *) &m_pZ80Ram[strAddr], endAddr - strAddr - 1);
            cout << message;
            cout.flush();
            break;
        }
        default:
        {
            cout << "BDOS Call " << cpu.getRegC() << endl;
            finish = true;
            cout << finish << endl;
        }
    }
    // opcode would be modified before the decodeOpcode method
    return opcode;
}
#endif

void Z80emu::runTest(std::ifstream* f) {
    streampos size;
    if (!f->is_open()) {
        cout << "file NOT OPEN" << endl;
        return;
    } else cout << "file open" << endl;

    size = f->tellg();
    cout << "Test size: " << size << endl;
    f->seekg(0, ios::beg);
    f->read((char *) &m_pZ80Ram[0x100], size);
    f->close();

    cpu.reset();
    finish = false;

    m_pZ80Ram[0] = (uint8_t) 0xC3;
    m_pZ80Ram[1] = 0x00;
    m_pZ80Ram[2] = 0x01; // JP 0x100 CP/M TPA
    m_pZ80Ram[5] = (uint8_t) 0xC9; // Return from BDOS call

#ifdef WITH_BREAKPOINT_SUPPORT
    cpu.setBreakpoint(0x0005, true);
#endif
    while (!finish) {
        cpu.execute();
    }
}


void Z80emu::initialise(unsigned char const* base, size_t size) {

    memcpy(&m_pZ80Ram[0x0000],  base,  size);
    cpu.reset();
}


// References:
//
// - http://rk.nvg.ntnu.no/sinclair/faq/fileform.html#SNA
//
// Load .SNA, .snap or .snapshot (Mirage Microdrive format used by many emulators)
//
// This format is the most well-supported of all snapshot formats (though Z80 is close on its heels) but has a drawback:
//
// As the program counter is pushed onto the stack so that a RETN instruction can restart the program, 2 bytes of memory
// are overwritten. This will usually not matter; the game (or whatever) will have stack space that can be used for this.
// However, if this space is all in use when the snap is made, memory below the stack space will be corrupted. According
// to Rui Ribeiro, the effects of this can sometimes be avoided by replacing the corrupted bytes with zeros; e.g. take
// the PC from the, stack pointer, replace that word with 0000 and then increment SP. This worked with snapshots of Batman,
// Bounder and others which had been saved at critical points. Theoretically, this problem could cause a complete crash on
// a real Spectrum if the stack pointer happened to be at address 16384; the push would try and write to the ROM. How
// different emulators handle this is not something I know...
//
// When the registers have been loaded, a RETN command is required to start the program. IFF2 is short for interrupt
// flip-flop 2, and for all practical purposes is the interrupt-enabled flag. Set means enabled.
//
//    Offset   Size   Description
//    ------------------------------------------------------------------------
//    0        1      byte   I
//    1        8      word   HL',DE',BC',AF'
//    9        10     word   HL,DE,BC,IY,IX
//    19       1      byte   Interrupt (bit 2 contains IFF2, 1=EI/0=DI)
//    20       1      byte   R
//    21       4      words  AF,SP
//    25       1      byte   IntMode (0=IM0/1=IM1/2=IM2)
//    26       1      byte   BorderColor (0..7, not used by Spectrum 1.7)
//    27       49152  bytes  RAM dump 16384..65535
//    ------------------------------------------------------------------------
//    Total: 49179 bytes
//
void Z80emu::loadSnapshot(const uint8_t* snapshot) {

    /* Skip the first 27 (0x1B) bytes of the snapshot to load the RAM dump into the last three 16K blocks of
     * ZX Spectrum memory.
     */
    memcpy(&m_pZ80Ram[0x4000],&snapshot[0x1B],  0xC000);

    cpu.reset();

    // then set the registers
    // $00  I
    // $01  HL'
    // $03  DE'
    // $05  BC'
    // $07  AF'
    // $09  HL
    // $0B  DE
    // $0D  BC
    // $0F  IY
    // $11  IX
    // $13  IFF2    [Only bit 2 is defined: 1 for EI, 0 for DI]
    // $14  R
    // $15  AF
    // $17  SP
    // $19  Interrupt mode: 0, 1 or 2
    // $1A  Border colour

    cpu.setRegI(snapshot[0x00]);

    cpu.setRegHLx(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x01])));
    cpu.setRegDEx(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x03])));
    cpu.setRegBCx(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x05])));
    cpu.setRegAFx(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x07])));
    cpu.setRegHL(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x09])));
    cpu.setRegDE(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x0B])));
    cpu.setRegBC(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x0D])));
    cpu.setRegIX(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x0F])));
    cpu.setRegIY(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x11])));


    bool isInterruptEnabled = (snapshot[0x13] & 0x04u) != 0;
    cpu.setIFF1(isInterruptEnabled);
    cpu.setIFF2(isInterruptEnabled);

    cpu.setRegR(snapshot[0x14]);
    cpu.setRegA(snapshot[0x16]);
    cpu.setFlags(snapshot[0x15]);

    // FIXME: swap twice?
    cpu.setRegSP(bswap16(bswap16(*reinterpret_cast<const uint16_t *>(&snapshot[0x17]))));

    switch (snapshot[0x19] & 0x03u) {
        case 0:
            cpu.setIM(Z80::IntMode::IM0);
            break;
        case 1:
            cpu.setIM(Z80::IntMode::IM1);
            break;
        case 2:
            cpu.setIM(Z80::IntMode::IM2);
            break;
    }

    // TODO: handle the border colour at location snapshot[0x1A]
    m_border = snapshot[0x1A];
    m_pZxDisplay->updateBorder(m_border, 0);

    //  Alternative way of generating a RETN instruction
    //  cpu.decodeED(0x7Du); // Issue a RETN instruction to pop the return address from the stack
    cpu.setRegPC(0x72); // dirección de RETN en la ROM
}

// void Z80emu::initialise(unsigned char const* base, size_t size) {

//     memcpy(&m_pZ80Ram[0x0100],  base,  size);
//     cpu.reset();

//     m_pZ80Ram[0] = (uint8_t) 0xC3;
//     m_pZ80Ram[1] = 0x00;
//     m_pZ80Ram[2] = 0x01; // JP 0x100 CP/M TPA
//     m_pZ80Ram[5] = (uint8_t) 0xC9; // Return from BDOS call

//     cpu.setBreakpoint(0x0005, true);
// }


//uint32_t Z80emu::run(const uint32_t tstates) {
//    while (this->m_tstates < tstates) {
//        cpu.execute();
//    }
//    return this->m_tstates;
//}


void Z80emu::execute(const uint32_t tstates) {

    Clock &clock = Clock::getInstance();
    while (clock.getTstates() < tstates) {
        cpu.execute();
    }
}


uint8_t *Z80emu::getRam() {
    return m_pZ80Ram;
}


uint32_t Z80emu::getStates() {
    return Clock::getInstance().getTstates();
}


void Z80emu::resetStates() {
    Clock::getInstance().reset();
}
