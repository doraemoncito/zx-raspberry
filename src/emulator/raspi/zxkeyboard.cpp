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
#include <cstring>
#include <cstdint>
#include <circle/logger.h>
#include <circle/string.h>
#include "keyboard.h"
#include "zxkeyboard.h"
#include "Z80emu.h"

/* Using shift keys and a combination of modes the Spectrum 40-key keyboard can be mapped to 256 input characters
 *
 * ---------------------------------------------------------------------------
 *
 *         0     1     2     3     4 -Bits-  4     3     2     1     0
 * PORT                                                                    PORT
 *
 * F7FE  [ 1 ] [ 2 ] [ 3 ] [ 4 ] [ 5 ]  |  [ 6 ] [ 7 ] [ 8 ] [ 9 ] [ 0 ]   EFFE
 *  ^                                   |                                   v
 * FBFE  [ Q ] [ W ] [ E ] [ R ] [ T ]  |  [ Y ] [ U ] [ I ] [ O ] [ P ]   DFFE
 *  ^                                   |                                   v
 * FDFE  [ A ] [ S ] [ D ] [ F ] [ G ]  |  [ H ] [ J ] [ K ] [ L ] [ ENT ] BFFE
 *  ^                                   |                                   v
 * FEFE  [SHI] [ Z ] [ X ] [ C ] [ V ]  |  [ B ] [ N ] [ M ] [sym] [ SPC ] 7FFE
 *  ^     $27                                                 $18           v
 * Start                                                                   End
 *        00100111                                            00011000
 *
 * ---------------------------------------------------------------------------
 *
 * The neat arrangement of ports means that the B register need only be rotated left to work up the left-hand side and
 * then down the right-hand side of the keyboard. When the reset bit drops into the carry then all 8 half-rows have
 * been read. Shift is the first key to be read. The lower six bits of the shifts are unambiguous.
 *
 * reference: https://web.archive.org/web/20150825085532/http://www.wearmouth.demon.co.uk/zx82.htm
 */

static const char FromKeyboard[] = "Keyboard";

void ZxKeyboard::keyStatusHandlerRaw(Z80emu &zxRaspberry, unsigned char ucModifiers, const unsigned char rawKeys[6]) {

    CString Message;
    Message.Format("Key status (modifiers %02X)", (unsigned) ucModifiers);

    uint8_t port_F7FE = 0xFFu;
    uint8_t port_FBFE = 0xFFu;
    uint8_t port_FDFE = 0xFFu;
    uint8_t port_FEFE = 0xFFu;
    uint8_t port_EFFE = 0xFFu;
    uint8_t port_DFFE = 0xFFu;
    uint8_t port_BFFE = 0xFFu;
    uint8_t port_7FFE = 0xFFu;

#ifdef DEBUG
    boolean hasValue = false;
#endif // DEBUG

    switch (ucModifiers) {
        case 0x02: // Left shift key -> CAPS SHIFT
        case 0x20: // Right shift key -> CAPS SHIFT
#ifdef DEBUG
            CLogger::Get()->Write(FromKeyboard, LogNotice, "Modifier is 0x%02X 'CAPS SHIFT'", ucModifiers);
#endif // DEBUG
            port_FEFE &= KEY_PRESSED_BIT0;
#ifdef DEBUG
            hasValue = true;
#endif // DEBUG
            break;
        case 0x01: // Left control key -> SYMBOL SHIFT
        case 0x10: // Right control key -> SYMBOL SHIFT
        case 0x04: // Alt -> SYMBOL SHIFT
        case 0x40: // Alt Gr -> SYMBOL SHIFT
#ifdef DEBUG
            CLogger::Get()->Write(FromKeyboard, LogNotice, "Modifier is 0x%02X 'SYMBOL SHIFT'", ucModifiers);
#endif // DEBUG
            port_7FFE &= KEY_PRESSED_BIT1;
#ifdef DEBUG
            hasValue = true;
#endif // DEBUG
            break;
    }

    for (unsigned i = 0; i < 6; i++)
    {
        if (rawKeys[i] != 0) {
            CString KeyCode;
            KeyCode.Format(" %02X", (unsigned) rawKeys[i]);

            Message.Append(KeyCode);
#ifdef DEBUG
            hasValue = true;
#endif // DEBUG

            switch (rawKeys[i]) {
                // row 0xF7FE
                case 0x1E: // 1
                    port_F7FE &= KEY_PRESSED_BIT0;
                    break;
                case 0x1F: // 2
                    port_F7FE &= KEY_PRESSED_BIT1;
                    break;
                case 0x20: // 3
                    port_F7FE &= KEY_PRESSED_BIT2;
                    break;
                case 0x21: // 4
                    port_F7FE &= KEY_PRESSED_BIT3;
                    break;
                case 0x22: // 5 (left)
                    port_F7FE &= KEY_PRESSED_BIT4;
                    break;

                    // row 0xEFFE
                case 0x23: // 6 (down)
                    port_EFFE &= KEY_PRESSED_BIT4;
                    break;
                case 0x24: // 7 (up)
                    port_EFFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x25: // 8 (right)
                    port_EFFE &= KEY_PRESSED_BIT2;
                    break;
                case 0x26: // 9
                    port_EFFE &= KEY_PRESSED_BIT1;
                    break;
                case 0x27: // 0
                    port_EFFE &= KEY_PRESSED_BIT0;
                    break;

                    // row 0xFBFE
                case 0x14: // Q
                    port_FBFE &= KEY_PRESSED_BIT0;
                    break;
                case 0x1A: // W
                    port_FBFE &= KEY_PRESSED_BIT1;
                    break;
                case 0x08: // E
                    port_FBFE &= KEY_PRESSED_BIT2;
                    break;
                case 0x15: // R
                    port_FBFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x17: // T
                    port_FBFE &= KEY_PRESSED_BIT4;
                    break;

                    // row 0xDFFE
                case 0x1C: // Y
                    port_DFFE &= KEY_PRESSED_BIT4;
                    break;
                case 0x18: // U
                    port_DFFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x0C: // I
                    port_DFFE &= KEY_PRESSED_BIT2;
                    break;
                case 0x12: // O
                    port_DFFE &= KEY_PRESSED_BIT1;
                    break;
                case 0x13: // P
                    port_DFFE &= KEY_PRESSED_BIT0;
                    break;

                    // row 0xFDFE
                case 0x04: // A
                    port_FDFE &= KEY_PRESSED_BIT0;
                    break;
                case 0x16: // S
                    port_FDFE &= KEY_PRESSED_BIT1;
                    break;
                case 0x07: // D
                    port_FDFE &= KEY_PRESSED_BIT2;
                    break;
                case 0x09: // F
                    port_FDFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x0A: // G
                    port_FDFE &= KEY_PRESSED_BIT4;
                    break;

                    // row 0xBFFE
                case 0x0B: // H
                    port_BFFE &= KEY_PRESSED_BIT4;
                    break;
                case 0x0D: // J
                    port_BFFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x0E: // K
                    port_BFFE &= KEY_PRESSED_BIT2;
                    break;
                case 0x0F: // L
                    port_BFFE &= KEY_PRESSED_BIT1;
                    break;
                case 0x28: // ENTER
                    port_BFFE &= KEY_PRESSED_BIT0;
                    break;

                    // row 0xFEFE
                case 0x1D: // Z
                    port_FEFE &= KEY_PRESSED_BIT1;
                    break;
                case 0x1B: // X
                    port_FEFE &= KEY_PRESSED_BIT2;
                    break;
                case 0x06: // C
                    port_FEFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x19: // V
                    port_FEFE &= KEY_PRESSED_BIT4;
                    break;

                    // row 0x7FFE
                case 0x05: // B
                    port_7FFE &= KEY_PRESSED_BIT4;
                    break;
                case 0x11: // N
                    port_7FFE &= KEY_PRESSED_BIT3;
                    break;
                case 0x10: // M
                    port_7FFE &= KEY_PRESSED_BIT2;
                    break;
                    // SYMBOL SHIFT is handled as a modifier.  See code above.
                case 0x2C: // BREAK SPACE
                    port_7FFE &= KEY_PRESSED_BIT0;
                    break;
            }
        }
    }

    zxRaspberry.internalOutPort(0xF7FE, port_F7FE);
    zxRaspberry.internalOutPort(0xFBFE, port_FBFE);
    zxRaspberry.internalOutPort(0xFDFE, port_FDFE);
    zxRaspberry.internalOutPort(0xFEFE, port_FEFE);
    zxRaspberry.internalOutPort(0xEFFE, port_EFFE);
    zxRaspberry.internalOutPort(0xDFFE, port_DFFE);
    zxRaspberry.internalOutPort(0xBFFE, port_BFFE);
    zxRaspberry.internalOutPort(0x7FFE, port_7FFE);

#ifdef DEBUG
    if (hasValue) {
        CLogger::Get()->Write(FromKeyboard, LogNotice, Message);
    }
#endif // DEBUG
}
