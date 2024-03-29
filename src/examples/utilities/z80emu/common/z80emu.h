#ifndef Z80EMU_H
#define Z80EMU_H

#include <iostream>
#include <fstream>

#include "z80.h"
#include "z80operations.h"

class Z80emu : public Z80operations
{
private:
    uint64_t tstates;
    Z80 cpu;
    uint8_t z80Ram[0x10000];
    uint8_t z80Ports[0x10000];
    bool finish;

public:
    Z80emu(void);
    virtual ~Z80emu() override;

    uint8_t *getRam();

    uint8_t fetchOpcode(uint16_t address) override;
    uint8_t peek8(uint16_t address) override;
    void poke8(uint16_t address, uint8_t value) override;
    uint16_t peek16(uint16_t address) override;
    void poke16(uint16_t address, RegisterPair word) override;
    uint8_t inPort(uint16_t port) override;
    void outPort(uint16_t port, uint8_t value) override;
    void addressOnBus(uint16_t address, int32_t tstates) override;
    void interruptHandlingTime(int32_t tstates) override;
    bool isActiveINT(void) override;
    uint8_t breakpoint(uint16_t address, uint8_t opcode) override;
#ifdef WITH_EXEC_DONE
    void execDone(void) override;
#endif

    void runTest(std::ifstream* f);
    void initialise(unsigned char const* base, size_t size);
    void loadSnapshot(const uint8_t* snapshot);
    uint64_t run(uint64_t tstates);

};
#endif // Z80EMU_H
