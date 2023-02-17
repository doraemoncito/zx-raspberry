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
#ifndef SCREEN_H
#define SCREEN_H

#include <QImage>
#include <QKeyEvent>
#include <QWidget>
#include "common/zxdisplay.h"


class CBcmFrameBuffer;
class Z80emu;
class ZxKeyboard;

/* RENAME THIS CLASS TO EmulatorScreen */

class ZxEmulatorScreen : public QWidget {
Q_OBJECT

public:
    explicit ZxEmulatorScreen(Z80emu *z80emu, QWidget *parent = nullptr);
    ~ZxEmulatorScreen();

    [[nodiscard]] QSize minimumSizeHint() const override;
    [[nodiscard]] QSize sizeHint() const override;

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool antiAliased = false;
    bool showDialog = false;

    Z80emu *m_z80emu;
    ZxKeyboard *m_zxKeyboard;
    ZxDisplay m_zxDisplay;
    CBcmFrameBuffer *bcmFrameBuffer = nullptr;

    QImage image = QImage(352, 296, QImage::Format_RGB32);
};

#endif // SCREEN_H
