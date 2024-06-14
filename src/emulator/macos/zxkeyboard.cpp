/*
 * Copyright (c) 2020-2024 Jose Hernandez
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
#include <cstdint>
#include "common/keyboard.h"
#include "common/Z80emu.h"
#include "zxkeyboard.h"
#include <QtGlobal>
#include <QtDebug>
#include <QKeyEvent>

/* Using shift keys and a combination of modes, the Spectrum 40-key keyboard can be mapped to 256 input characters
 *
 * ---------------------------------------------------------------------------
 *
 *          0     1     2     3     4 -Bits-  4     3     2     1     0
 * PORT                                                                     PORT
 *
 * F7FE   [ 1 ] [ 2 ] [ 3 ] [ 4 ] [ 5 ]  |  [ 6 ] [ 7 ] [ 8 ] [ 9 ] [ 0 ]   EFFE
 *  ^                                    |                                   v
 * FBFE   [ Q ] [ W ] [ E ] [ R ] [ T ]  |  [ Y ] [ U ] [ I ] [ O ] [ P ]   DFFE
 *  ^                                    |                                   v
 * FDFE   [ A ] [ S ] [ D ] [ F ] [ G ]  |  [ H ] [ J ] [ K ] [ L ] [ ENT ] BFFE
 *  ^                                    |                                   v
 * FEFE  [ CS ] [ Z ] [ X ] [ C ] [ V ]  |  [ B ] [ N ] [ M ] [sym] [ SPC ] 7FFE
 *  ^     $27                                                 $18           v
 * Start                                                                   End
 *        00100111                                            00011000
 *
 * ---------------------------------------------------------------------------
 *
 * The neat arrangement of ports means that the B register need only be rotated left to work up the left-hand side and
 * then down the right-hand side of the keyboard. When the reset bit drops into the carry, then all 8 half-rows have
 * been read. Shift is the first key to be read. The lower six bits of the shifts are unambiguous.
 *
 * reference: https://web.archive.org/web/20150825085532/http://www.wearmouth.demon.co.uk/zx82.htm
 */

ZxKeyboard::ZxKeyboard() {
}


void ZxKeyboard::reset() {

    port_F7FE = 0xFFu;
    port_FBFE = 0xFFu;
    port_FDFE = 0xFFu;
    port_FEFE = 0xFFu;
    port_EFFE = 0xFFu;
    port_DFFE = 0xFFu;
    port_BFFE = 0xFFu;
    port_7FFE = 0xFFu;
}


void ZxKeyboard::keyPressEvent(Z80emu &zxRaspberry, QKeyEvent &event) {

//    qDebug() << "Received key press event" << &event;

    // Modifier keys pressed on their own
    switch (event.key()) {
        case Qt::Key_Shift:         // Left shift key -> CAPS SHIFT
            port_FEFE &= KEY_PRESSED_BIT0;
            break;
        case Qt::Key_Control:       // Command key -> SYMBOL SHIFT
        case Qt::Key_Meta:          // Control key -> SYMBOL SHIFT
        case Qt::Key_Alt:           // Option key -> SYMBOL SHIFT
        case Qt::Key_AltGr:         // Alt Gr -> SYMBOL SHIFT
            port_7FFE &= KEY_PRESSED_BIT1;
            break;
    }

    // Key modifiers when used in combination with other keys
    Qt::KeyboardModifiers modifiers = event.modifiers();

   if (modifiers & Qt::ShiftModifier)
       port_FEFE &= KEY_PRESSED_BIT0;

   if ((modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier) || (modifiers & Qt::MetaModifier))
       port_7FFE &= KEY_PRESSED_BIT1;

    switch (event.key()) {
        // row 0xF7FE
        case Qt::Key_1: // 1
            port_F7FE &= KEY_PRESSED_BIT0;
            break;
        case Qt::Key_2: // 2
            port_F7FE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_3: // 3
            port_F7FE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_4: // 4
            port_F7FE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_Left:
        case Qt::Key_5: // 5 (left)
            port_F7FE &= KEY_PRESSED_BIT4;
            break;

        // row 0xEFFE
        case Qt::Key_Down:
        case Qt::Key_6: // 6 (down)
            port_EFFE &= KEY_PRESSED_BIT4;
            break;
        case Qt::Key_Up:
        case Qt::Key_7: // 7 (up)
            port_EFFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_Right:
        case Qt::Key_8: // 8 (right)
            port_EFFE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_9: // 9
            port_EFFE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_0: // 0
            port_EFFE &= KEY_PRESSED_BIT0;
            break;

        // row 0xFBFE
        case Qt::Key_Q: // Q
            port_FBFE &= KEY_PRESSED_BIT0;
            break;
        case Qt::Key_W: // W
            port_FBFE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_E: // E
            port_FBFE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_R: // R
            port_FBFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_T: // T
            port_FBFE &= KEY_PRESSED_BIT4;
            break;

        // row 0xDFFE
        case Qt::Key_Y: // Y
            port_DFFE &= KEY_PRESSED_BIT4;
            break;
        case Qt::Key_U: // U
            port_DFFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_I: // I
            port_DFFE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_O: // O
            port_DFFE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_P: // P
            port_DFFE &= KEY_PRESSED_BIT0;
            break;

        // row 0xFDFE
        case Qt::Key_A: // A
            port_FDFE &= KEY_PRESSED_BIT0;
            break;
        case Qt::Key_S: // S
            port_FDFE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_D: // D
            port_FDFE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_F: // F
            port_FDFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_G: // G
            port_FDFE &= KEY_PRESSED_BIT4;
            break;

        // row 0xBFFE
        case Qt::Key_H: // H
            port_BFFE &= KEY_PRESSED_BIT4;
            break;
        case Qt::Key_J: // J
            port_BFFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_K: // K
            port_BFFE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_L: // L
            port_BFFE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter: // ENTER
            port_BFFE &= KEY_PRESSED_BIT0;
            break;

        // row 0xFEFE
        case Qt::Key_Z: // Z
            port_FEFE &= KEY_PRESSED_BIT1;
            break;
        case Qt::Key_X: // X
            port_FEFE &= KEY_PRESSED_BIT2;
            break;
        case Qt::Key_C: // C
            port_FEFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_V: // V
            port_FEFE &= KEY_PRESSED_BIT4;
            break;

        // row 0x7FFE
        case Qt::Key_B: // B
            port_7FFE &= KEY_PRESSED_BIT4;
            break;
        case Qt::Key_N: // N
            port_7FFE &= KEY_PRESSED_BIT3;
            break;
        case Qt::Key_M: // M
            port_7FFE &= KEY_PRESSED_BIT2;
            break;
        // SYMBOL SHIFT is handled as a modifier.  See code above.
        case Qt::Key_Space: // BREAK SPACE
            port_7FFE &= KEY_PRESSED_BIT0;
            break;
    }

    zxRaspberry.internalOutPort(0xF7FE, port_F7FE);
    zxRaspberry.internalOutPort(0xFBFE, port_FBFE);
    zxRaspberry.internalOutPort(0xFDFE, port_FDFE);
    zxRaspberry.internalOutPort(0xFEFE, port_FEFE);
    zxRaspberry.internalOutPort(0xEFFE, port_EFFE);
    zxRaspberry.internalOutPort(0xDFFE, port_DFFE);
    zxRaspberry.internalOutPort(0xBFFE, port_BFFE);
    zxRaspberry.internalOutPort(0x7FFE, port_7FFE);
}


void ZxKeyboard::keyReleaseEvent(Z80emu &zxRaspberry, QKeyEvent &event) {

//    qDebug() << "Received key release event" << &event;

    // Modifier keys pressed on their own
    switch (event.key()) {
        case Qt::Key_Shift:         // Left shift key -> CAPS SHIFT
            port_FEFE |= ~KEY_PRESSED_BIT0;
            break;
        case Qt::Key_Control:       // Command key -> SYMBOL SHIFT
        case Qt::Key_Meta:          // Control key -> SYMBOL SHIFT
        case Qt::Key_Alt:           // Option key -> SYMBOL SHIFT
        case Qt::Key_AltGr:         // Alt Gr -> SYMBOL SHIFT
            port_7FFE |= ~KEY_PRESSED_BIT1;
            break;
    }

    // Key modifiers when used in combination with other keys
    Qt::KeyboardModifiers modifiers = event.modifiers();

    if (modifiers & Qt::ShiftModifier)
        port_FEFE |= ~KEY_PRESSED_BIT0;

    if ((modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier) || (modifiers & Qt::MetaModifier))
        port_7FFE |= ~KEY_PRESSED_BIT1;

    switch (event.key()) {
        // row 0xF7FE
        case Qt::Key_1: // 1
            port_F7FE |= ~KEY_PRESSED_BIT0;
            break;
        case Qt::Key_2: // 2
            port_F7FE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_3: // 3
            port_F7FE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_4: // 4
            port_F7FE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_Left:
        case Qt::Key_5: // 5 (left)
            port_F7FE |= ~KEY_PRESSED_BIT4;
            break;

            // row 0xEFFE
        case Qt::Key_Down:
        case Qt::Key_6: // 6 (down)
            port_EFFE |= ~KEY_PRESSED_BIT4;
            break;
        case Qt::Key_Up:
        case Qt::Key_7: // 7 (up)
            port_EFFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_Right:
        case Qt::Key_8: // 8 (right)
            port_EFFE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_9: // 9
            port_EFFE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_0: // 0
            port_EFFE |= ~KEY_PRESSED_BIT0;
            break;

            // row 0xFBFE
        case Qt::Key_Q: // Q
            port_FBFE |= ~KEY_PRESSED_BIT0;
            break;
        case Qt::Key_W: // W
            port_FBFE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_E: // E
            port_FBFE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_R: // R
            port_FBFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_T: // T
            port_FBFE |= ~KEY_PRESSED_BIT4;
            break;

            // row 0xDFFE
        case Qt::Key_Y: // Y
            port_DFFE |= ~KEY_PRESSED_BIT4;
            break;
        case Qt::Key_U: // U
            port_DFFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_I: // I
            port_DFFE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_O: // O
            port_DFFE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_P: // P
            port_DFFE |= ~KEY_PRESSED_BIT0;
            break;

            // row 0xFDFE
        case Qt::Key_A: // A
            port_FDFE |= ~KEY_PRESSED_BIT0;
            break;
        case Qt::Key_S: // S
            port_FDFE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_D: // D
            port_FDFE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_F: // F
            port_FDFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_G: // G
            port_FDFE |= ~KEY_PRESSED_BIT4;
            break;

            // row 0xBFFE
        case Qt::Key_H: // H
            port_BFFE |= ~KEY_PRESSED_BIT4;
            break;
        case Qt::Key_J: // J
            port_BFFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_K: // K
            port_BFFE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_L: // L
            port_BFFE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter: // ENTER
            port_BFFE |= ~KEY_PRESSED_BIT0;
            break;

            // row 0xFEFE
        case Qt::Key_Z: // Z
            port_FEFE |= ~KEY_PRESSED_BIT1;
            break;
        case Qt::Key_X: // X
            port_FEFE |= ~KEY_PRESSED_BIT2;
            break;
        case Qt::Key_C: // C
            port_FEFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_V: // V
            port_FEFE |= ~KEY_PRESSED_BIT4;
            break;

            // row 0x7FFE
        case Qt::Key_B: // B
            port_7FFE |= ~KEY_PRESSED_BIT4;
            break;
        case Qt::Key_N: // N
            port_7FFE |= ~KEY_PRESSED_BIT3;
            break;
        case Qt::Key_M: // M
            port_7FFE |= ~KEY_PRESSED_BIT2;
            break;
            // SYMBOL SHIFT is handled as a modifier.  See code above.
        case Qt::Key_Space: // BREAK SPACE
            port_7FFE |= ~KEY_PRESSED_BIT0;
            break;
    }

    zxRaspberry.internalOutPort(0xF7FE, port_F7FE);
    zxRaspberry.internalOutPort(0xFBFE, port_FBFE);
    zxRaspberry.internalOutPort(0xFDFE, port_FDFE);
    zxRaspberry.internalOutPort(0xFEFE, port_FEFE);
    zxRaspberry.internalOutPort(0xEFFE, port_EFFE);
    zxRaspberry.internalOutPort(0xDFFE, port_DFFE);
    zxRaspberry.internalOutPort(0xBFFE, port_BFFE);
    zxRaspberry.internalOutPort(0x7FFE, port_7FFE);
}
