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
#include <circle/bcmframebuffer.h>
#include <circle/logger.h>
#include <circle/startup.h>
#include <circle/usb/usbgamepad.h>
#include "common/gui/zxlabel.h"
#include "z80.h"
#include "Z80emu.h"
//#include "z80operations.h"
#include "zxula.h"
#include "keyboard.h"

static const char FromUla[] = "ULA";

ZxUla::ZxUla(Z80emu &zxRaspberry, CBcmFrameBuffer &frameBuffer) : m_zxRaspberry(zxRaspberry), m_frameBuffer(frameBuffer) {

    isSwitchingGamePadAdapter = false;
}

void ZxUla::scanLineReset() {
    scanLineIndex = 0;
}

void ZxUla::scanLineNext() {
    uint8_t border = m_zxRaspberry.getBorder();
    reinterpret_cast<uint8_t *>(m_frameBuffer.GetBuffer())[scanLineIndex] = (border << 4) | (border & 0x0F);
    scanLineIndex++;
}


void ZxUla::refreshInit() {

    port_011F = 0x00u;
    port_F7FE = 0xFFu;
    port_FBFE = 0xFFu;
    port_FDFE = 0xFFu;
    port_FEFE = 0xFFu;
    port_EFFE = 0xFFu;
    port_DFFE = 0xFFu;
    port_BFFE = 0xFFu;
    port_7FFE = 0xFFu;

//    m_zxRaspberry.internalOutPort(0x011F, 0x00u);
//    m_zxRaspberry.internalOutPort(0xF7FE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0xFBFE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0xFDFE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0xFEFE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0xEFFE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0xDFFE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0xBFFE, 0xFFu);
//    m_zxRaspberry.internalOutPort(0x7FFE, 0xFFu);
}

void ZxUla::refreshDone() {
    refreshPort(0x011F, port_011F);
    refreshPort(0xF7FE, port_F7FE);
    refreshPort(0xFBFE, port_FBFE);
    refreshPort(0xFDFE, port_FDFE);
    refreshPort(0xFEFE, port_FEFE);
    refreshPort(0xEFFE, port_EFFE);
    refreshPort(0xDFFE, port_DFFE);
    refreshPort(0xBFFE, port_BFFE);
    refreshPort(0x7FFE, port_7FFE);
}

void ZxUla::refreshPort(uint16_t port, uint8_t value) {
#ifdef DEBUG
    if ((((port & 0x00FE) == 0x00FE) && (value != 0xFFu)) || ((port == 0x011F) && (value != 0x00u))) {
        CString Message;
        Message.Format("Updating port '%04X' = '%02X'", port, value);
        CLogger::Get()->Write(FromUla, LogNotice, Message);
    }
#endif // DEBUG
    m_zxRaspberry.internalOutPort(port, value);
}

//   Using shift keys and a combination of modes the Spectrum 40-key keyboard
//   can be mapped to 256 input characters
//
// ---------------------------------------------------------------------------
//
//         0     1     2     3     4 -Bits-  4     3     2     1     0
// PORT                                                                    PORT
//
// F7FE  [ 1 ] [ 2 ] [ 3 ] [ 4 ] [ 5 ]  |  [ 6 ] [ 7 ] [ 8 ] [ 9 ] [ 0 ]   EFFE
//  ^                                   |                                   v
// FBFE  [ Q ] [ W ] [ E ] [ R ] [ T ]  |  [ Y ] [ U ] [ I ] [ O ] [ P ]   DFFE
//  ^                                   |                                   v
// FDFE  [ A ] [ S ] [ D ] [ F ] [ G ]  |  [ H ] [ J ] [ K ] [ L ] [ ENT ] BFFE
//  ^                                   |                                   v
// FEFE  [SHI] [ Z ] [ X ] [ C ] [ V ]  |  [ B ] [ N ] [ M ] [sym] [ SPC ] 7FFE
//  ^     $27                                                 $18           v
// Start                                                                   End
//        00100111                                            00011000
//
// ---------------------------------------------------------------------------
//   The above map may help in reading.
//   The neat arrangement of ports means that the B register need only be
//   rotated left to work up the left hand side and then down the right
//   hand side of the keyboard. When the reset bit drops into the carry
//   then all 8 half-rows have been read. Shift is the first key to be
//   read. The lower six bits of the shifts are unambiguous.
//
// reference: https://web.archive.org/web/20150825085532/http://www.wearmouth.demon.co.uk/zx82.htm
//
void ZxUla::refreshKeyboard(unsigned char ucModifiers, const unsigned char RawKeys[6]) {
#ifdef DEBUG
    CString Message;
    Message.Format("Key status (modifiers 0x%02X)", (unsigned) ucModifiers);
#endif // DEBUG

    // call the keyboard adapter using ZXOperations rather than zxRaspberry
    boolean hasValue = false;

    // Detect Ctrl+Alt+Delete and reboot the device
    if ((ucModifiers == 0x05) && (RawKeys[0] == 0x63)) {
        CLogger::Get()->Write(FromUla, LogNotice, "Ctrl+Alt+Delete detected: rebooting device");
        reboot();
    }

    switch (ucModifiers) {
        case 0x02: // Left shift key -> CAPS SHIFT
        case 0x20: // Right shift key -> CAPS SHIFT
#ifdef DEBUG
            CLogger::Get()->Write(FromUla, LogNotice, "Modifier is 0x%02X 'CAPS SHIFT'", ucModifiers);
#endif // DEBUG
            port_FEFE &= KEY_PRESSED_BIT0;
            hasValue = true;
            break;
        case 0x01: // Left control key -> SYMBOL SHIFT
        case 0x10: // Right control key -> SYMBOL SHIFT
        case 0x04: // Alt -> SYMBOL SHIFT
        case 0x40: // Alt Gr -> SYMBOL SHIFT
#ifdef DEBUG
            CLogger::Get()->Write(FromUla, LogNotice, "Modifier is 0x%02X 'SYMBOL SHIFT'", ucModifiers);
#endif // DEBUG
            port_7FFE &= KEY_PRESSED_BIT1;
            hasValue = true;
            break;
    }

    for (unsigned i = 0; i < 6; i++) {
        if (RawKeys[i] != 0) {
            CString KeyCode;
            KeyCode.Format(" 0x%02X", (unsigned) RawKeys[i]);

#ifdef DEBUG
            Message.Append(KeyCode);
#endif // DEBUG
            hasValue = true;

            switch (RawKeys[i]) {
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

#ifdef DEBUG
    if (hasValue) {
        CLogger::Get()->Write(FromUla, LogNotice, Message);
    }
#endif // DEBUG
}

// http://www.retroisle.com/general/spectrum_joysticks.php
// Joystick emulation disabled
// Joystick KEMPSTON selected
// Joystick FULLER selected
// Joystick SINCLAIR 1 selected
// Joystick SINCLAIR 2 selected
// Joystick CURSOR/AGF/PROTEK selected
// Joystick Q-A-O-P-[SPACE]-M emulation selected
enum ZxGamePadMode {
    DISABLED,
    KEMPSTON,
    FULLER,
    SINCLAIR_1,
    SINCLAIR_2,
    CURSOR_AGF_PROTEK,
    Q_A_O_P_SPACE_M
};

void ZxUla::refreshGamepad(unsigned nDeviceIndex, const TGamePadState &pState) {

#ifdef DEBUG
    if (pState.buttons != 0) {
        CString Message;
        Message.Format("Gamepad button status: 0x%04X", (unsigned) pState.buttons);
        CLogger::Get()->Write(FromUla, LogNotice, Message);
    }
#endif // DEBUG

    // The 'share' button on a PS4 controller makes the emulator cycle through joystick emulation modes
//    if (isAxisButtonPressed(pState, GamePadButtonShare, GamePadAxisButtonUp)) {
    if (isAxisButtonPressed(pState, GamePadButtonPS, GamePadAxisButtonUp)) {
        if (!isSwitchingGamePadAdapter) {
            currentGamePadAdapter = (++currentGamePadAdapter >= m_gamepadAdapter.size()) ? 0 : currentGamePadAdapter;

            CString message;
            if (m_gamepadAdapter[currentGamePadAdapter]->name().empty()) {
                message.Format("Joystick emulation disabled");
            } else {
                message.Format("%s joystick selected", m_gamepadAdapter[currentGamePadAdapter]->name().c_str());
            }
            CLogger::Get()->Write(FromUla, LogNotice, message);

            // Message to be displayed with 3 spaces to the left in green ink with black background?
            // TODO: check this against ZX [Baremulator](http://zxmini.speccy.org/en/index.html) and see if honouring
            // the current background is better Message also disappears after a few seconds.
            auto zxLabel = ZxLabel(ZxRect(4, 32, 36, 1), message);
            zxLabel.draw(reinterpret_cast<uint8_t *>(m_frameBuffer.GetBuffer()), 0x4, 0x0);

            isSwitchingGamePadAdapter = true;
        }
    } else {
        isSwitchingGamePadAdapter = false;
    }

    // Kempston Joystick bit pattern: 000FUDLR
    // https://worldofspectrum.org/faq/reference/peripherals.htm
    // https://chuntey.wordpress.com/2010/01/06/using-kempston-joystick-in-your-own-basic-programs/
    port_011F = 0x00u;

    // Cursor Joystick emulation
    if (isAxisButtonPressed(pState, GamePadButtonUp, GamePadAxisButtonUp)) {
        // key 7 (up)
        port_EFFE &= KEY_PRESSED_BIT3;

        // Kempston Joystick (up)
        port_011F |= 0x08u;
    }
    if (isAxisButtonPressed(pState, GamePadButtonLeft, GamePadAxisButtonLeft)) {
        // key 5 (left)
        port_F7FE &= KEY_PRESSED_BIT4;

        // Kempston Joystick (left)
        port_011F |= 0x02u;
    }
    if (isAxisButtonPressed(pState, GamePadButtonRight, GamePadAxisButtonRight)) {
        // key 8 (right)
        port_EFFE &= KEY_PRESSED_BIT2;

        // Kempston Joystick (right)
        port_011F |= 0x01u;
    }
    if (isAxisButtonPressed(pState, GamePadButtonDown, GamePadAxisButtonDown)) {
        // key 6 (down)
        port_EFFE &= KEY_PRESSED_BIT4;

        // Kempston Joystick (down)
        port_011F |= 0x04u;
    }

    if (isAxisButtonPressed(pState, GamePadButtonTriangle, GamePadAxisButtonTriangle)) {
        // TODO: map this button to a key

        // Kempston Joystick (fire)
        port_011F |= 0x10u;
    }

    if (isAxisButtonPressed(pState, GamePadButtonSquare, GamePadAxisButtonSquare)) {
        // key BREAK SPACE
        port_7FFE &= KEY_PRESSED_BIT0;

        // Kempston Joystick (fire)
        port_011F |= 0x10u;
    }

    if (isAxisButtonPressed(pState, GamePadButtonCircle, GamePadAxisButtonCircle)) {
        // key ENTER
        port_BFFE &= KEY_PRESSED_BIT0;

        // Kempston Joystick (fire)
        port_011F |= 0x10u;
    }

    if (isAxisButtonPressed(pState, GamePadButtonCross, GamePadAxisButtonCross)) {
        // key 0 (cross)
        port_EFFE &= KEY_PRESSED_BIT0;

        // Kempston Joystick (fire)
        port_011F |= 0x10u;
    }

#ifdef DEBUG
    if (port_011F != 0x00u) {
        CString Message;
        Message.Format("Kempston joystick status at port 0x011F: 0x%02X", (unsigned) port_011F);
        CLogger::Get()->Write(FromUla, LogNotice, Message);
    }
#endif
}


bool ZxUla::isAxisButtonReleased(const TGamePadState &pState, TGamePadButton button, TGamePadAxis axis) {

    return !isAxisButtonPressed(pState, button, axis);
}

bool ZxUla::isAxisButtonPressed(const TGamePadState &pState, TGamePadButton button, TGamePadAxis axis) {

//    if (axis < m_GamePadState.naxes) {
//        CString Value;
//        Value.Format("%03u", m_GamePadState.axes[axis].value);
////        PrintAt(nPosX, nPosY, Value, m_GamePadState.buttons & button ? TRUE : FALSE);
////    } else {
////        PrintAt(nPosX, nPosY, pName, m_GamePadState.buttons & button ? TRUE : FALSE);
//    }

    return (pState.buttons & button) != 0;
}
