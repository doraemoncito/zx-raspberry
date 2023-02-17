#include <streambuf>
#include <istream>
#include <string.h>
#include <circle/logger.h>
#include "z80emu.h"


struct membuf: std::streambuf {
    membuf(unsigned char const* base, size_t size) {
        char* p(const_cast<char*>(reinterpret_cast<const char *>(base)));
        this->setg(p, p, p + size);
    }
};

struct imemstream: virtual membuf, std::istream {
    imemstream(unsigned char const* base, size_t size)
        : membuf(base, size)
        , std::istream(static_cast<std::streambuf*>(this)) {
    }
};

using namespace std;


static const char msgFromULA[] = "ULA";


Z80emu::Z80emu(void) : cpu(this)
{
    // TODO: Implement keyboard support. For now, we'll turn the keyboard off until we implement it.
    // Bits are set to 0 for any key that is pressed and 1 for any key that is not pressed. Multiple key presses can be read simultaneously.
    // http://www.breakintoprogram.co.uk/computers/zx-spectrum/keyboard
    z80Ports[0xFEFE] = 0xFF;
    z80Ports[0xFDFE] = 0xFF;
    z80Ports[0xFBFE] = 0xFF;
    z80Ports[0xF7FE] = 0xFF;
    z80Ports[0xEFFE] = 0xFF;
    z80Ports[0xDFFE] = 0xFF;
    z80Ports[0xBFFE] = 0xFF;
    z80Ports[0x7FFE] = 0xFF;
}

Z80emu::~Z80emu() {}

uint8_t Z80emu::fetchOpcode(uint16_t address) {
    // 3 clocks to fetch opcode from RAM and 1 execution clock = 4 t-states
    tstates += 4;
    return z80Ram[address];
}

uint8_t Z80emu::peek8(uint16_t address) {
    // 3 clocks for read byte from RAM
    tstates += 3;
    return z80Ram[address];
}

void Z80emu::poke8(uint16_t address, uint8_t value) {
    // 3 clocks for write byte to RAM
    tstates += 3;
    z80Ram[address] = value;
}

uint16_t Z80emu::peek16(uint16_t address) {
    // Order matters, first read lsb, then read msb, don't "optimize"
    uint8_t lsb = peek8(address);
    uint8_t msb = peek8(address + 1);
    return (msb << 8) | lsb;
}

void Z80emu::poke16(uint16_t address, RegisterPair word) {
    // Order matters, first write lsb, then write msb, don't "optimize"
    poke8(address, word.byte8.lo);
    poke8(address + 1, word.byte8.hi);
}

uint8_t Z80emu::inPort(uint16_t port) {
    // 4 clocks for read byte from bus
    tstates += 4;

    // TODO: ports 0xXXFE are reserved for the ZX spectrum ULA so we'll need a ULA class to deal with this
    if ((port & 0x00FF) == 0x00FE) {  // ZX spectrum ULA allocated port
        switch (port) {
            case 0xFEFE:
            case 0xFDFE:
            case 0xFBFE:
            case 0xF7FE:
            case 0xEFFE:
            case 0xDFFE:
            case 0xBFFE:
            case 0x7FFE:
                // TODO: read keyboard
                z80Ports[port] = 0xFF;
                break;
            case 0x00FE:
                // https://neuro.me.uk/projects/wos/sinclairfaq.dev/ng/cssfaq/reference/48kreference.htm
                z80Ports[port] = 0xFF;
                break;
            default:
                // Debug only those ports we are not handling yet
                CLogger::Get()->Write(msgFromULA, LogDebug, "[ULA IN  ] port 0x%04X <-- value 0x%02X", port, z80Ports[port]);
        }
    } else {
        CLogger::Get()->Write(msgFromULA, LogDebug, "[PORT IN ] port 0x%04X <-- value 0x%02X", port, z80Ports[port]);
    }

    return z80Ports[port];
}

void Z80emu::outPort(uint16_t port, uint8_t value) {
    // 4 clocks for write byte to bus
    tstates += 4;

    // according to https://faqwiki.zxnet.co.uk/wiki/ZX_Spectrum_ULA
    // OUT to port xxFE (the high byte is ignored) will set the border colour to {d2, d1, d0},
    // drive the MIC socket with d3 and the loudspeaker with d4.(again, polarity?) d5–d7 are not used.
    if ((port & 0x00FF) == 0x00FE) {  // ZX spectrum ULA allocated port
        switch (port) {
            // TODO: ports 0xXXFE are reserved for the ZX spectrum ULA so we'll need a ULA class to deal with this
            case 0x10FE:
                // (used in automania) what does this port do?
                break;
            case 0x02FE:
                // (used in dingo) what does this port do?
                break;
            case 0x12FE:
                // (used in dingo) what does this port do?
                break;
            case 0x00FE:
                // https://neuro.me.uk/projects/wos/sinclairfaq.dev/ng/cssfaq/reference/48kreference.htm
                break;
            default:
                // Debug only those ports we are not handling yet
                CLogger::Get()->Write(msgFromULA, LogDebug, "[PORT OUT] value 0x%02X --> port 0x%04X", value, port);
        }
    } else {
        CLogger::Get()->Write(msgFromULA, LogDebug, "[PORT OUT] value 0x%02X --> port 0x%04X", value, port);
    }

    z80Ports[port] = value;
}

void Z80emu::addressOnBus(uint16_t address, int32_t tstates) {
    // Additional clocks to be added on some instructions
    this->tstates += tstates;
}

void Z80emu::interruptHandlingTime(int32_t tstates) {
    this->tstates += tstates;
}

bool Z80emu::isActiveINT(void) {
	// Put here the needed logic to trigger an INT
    return false;
}

#ifdef WITH_EXEC_DONE
void Z80emu::execDone(void) {}
#endif

uint8_t Z80emu::breakpoint(uint16_t address, uint8_t opcode) {
    // Emulate CP/M Syscall at address 5
    switch (cpu.getRegC()) {
        case 0: // BDOS 0 System Reset
        {
            cout << "Z80 reset after " << tstates << " t-states" << endl;
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
            while (z80Ram[endAddr++] != '$');
            std::string message((const char *) &z80Ram[strAddr], endAddr - strAddr - 1);
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

void Z80emu::runTest(std::ifstream* f) {
    streampos size;
    if (!f->is_open()) {
        cout << "file NOT OPEN" << endl;
        return;
    } else cout << "file open" << endl;

    size = f->tellg();
    cout << "Test size: " << size << endl;
    f->seekg(0, ios::beg);
    f->read((char *) &z80Ram[0x100], size);
    f->close();

    cpu.reset();
    finish = false;

    z80Ram[0] = (uint8_t) 0xC3;
    z80Ram[1] = 0x00;
    z80Ram[2] = 0x01; // JP 0x100 CP/M TPA
    z80Ram[5] = (uint8_t) 0xC9; // Return from BDOS call

    cpu.setBreakpoint(0x0005, true);
    while (!finish) {
        cpu.execute();
    }
}


void Z80emu::initialise(unsigned char const* base, size_t size) {

    memcpy(&z80Ram[0x0000],  base,  size);
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

    // Skip the first 27 bytes of the snapshot to load RAM dump into the last three 16K blocks of ZX spectrum memory
    memcpy(&z80Ram[0x4000],  &snapshot[0x1B],  0xC000);
    cpu.resetSnapshot(snapshot);
}


// void Z80emu::initialise(unsigned char const* base, size_t size) {

//     memcpy(&z80Ram[0x0100],  base,  size);
//     cpu.reset();

//     z80Ram[0] = (uint8_t) 0xC3;
//     z80Ram[1] = 0x00;
//     z80Ram[2] = 0x01; // JP 0x100 CP/M TPA
//     z80Ram[5] = (uint8_t) 0xC9; // Return from BDOS call

//     cpu.setBreakpoint(0x0005, true);
// }


uint64_t Z80emu::run(uint64_t tstates) {
    tstates += this->m_tstates;
    while((tstates -= this->m_tstates) > 0) {
        cpu.execute();
    }
    this->m_tstates = tstates;
    return tstates;
}


uint8_t *Z80emu::getRam() {
    return z80Ram;
}
