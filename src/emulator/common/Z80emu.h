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
#ifndef Z80EMU_H
#define Z80EMU_H

#include <iostream>
#include <fstream>

#include "z80.h"
#include "z80operations.h"

class ZxDisplay;

class Z80emu : public Z80operations
{
private:
    Z80 cpu;
    uint8_t *m_pZ80Ram;
    uint8_t *m_pZ80Ports;
    bool finish;
    uint8_t m_border;
    ZxDisplay *m_pZxDisplay;

public:
    explicit Z80emu(ZxDisplay *pZxDisplay);
    ~Z80emu() override;

    uint8_t *getRam();

    uint8_t fetchOpcode(uint16_t address) override;
    uint8_t peek8(uint16_t address) override;
    void poke8(uint16_t address, uint8_t value) override;
    uint16_t peek16(uint16_t address) override;
    void poke16(uint16_t address, RegisterPair word) override;
    uint8_t inPort(uint16_t port) override;
    void outPort(uint16_t port, uint8_t value) override;
    void internalOutPort(uint16_t port, uint8_t value);
    void addressOnBus(uint16_t address, int32_t wstates) override;
    // Clocks needed for processing INT and NMI
    void interruptHandlingTime(int32_t wstates) override;
    bool isActiveINT() override;

#ifdef WITH_BREAKPOINT_SUPPORT
    // Callback for notify at PC address
    virtual uint8_t breakpoint(uint16_t address, uint8_t opcode) override;
#endif

#ifdef WITH_EXEC_DONE
    void execDone(void) override;
#endif

    void runTest(std::ifstream* f);
    void initialise(const uint8_t * const base, size_t size);
    void loadSnapshot(const uint8_t * const snapshot, size_t size);

    void execute(uint32_t);

    [[nodiscard]] uint8_t getBorder() const {
        return m_border;
    }

private:
    void preIO(int port);

    uint8_t *m_pDelayTstates;
    bool m_contendedRamPage[4];
    bool m_contendedIOPage[4];

};

#endif // Z80EMU_H
