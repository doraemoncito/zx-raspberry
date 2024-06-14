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
#ifndef SCREEN_H
#define SCREEN_H

#include <QImage>
#include <QKeyEvent>
#include <QWidget>
#include "common/zxdisplay.h"


class CBcmFrameBuffer;
class Z80emu;
class ZxKeyboard;

class ZxEmulatorScreen : public QWidget {
Q_OBJECT

public:
    explicit ZxEmulatorScreen(Z80emu *z80emu, ZxDisplay *pZxDisplay, QWidget *parent = nullptr);
    ~ZxEmulatorScreen() override;

    [[nodiscard]] QSize minimumSizeHint() const override;
    [[nodiscard]] QSize sizeHint() const override;

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    bool m_antiAliased = false;
    bool m_showDialog = false;
    uint8_t m_scale = 3;
    bool m_flash = false;
    uint32_t m_frameCounter = 0;

    Z80emu *m_pZ80emu;
    ZxKeyboard *m_pZxKeyboard;
    ZxDisplay *m_pZxDisplay;
    CBcmFrameBuffer *m_pBcmFrameBuffer = nullptr;

    QImage image = QImage(ZxDisplay::DISPLAY_WIDTH, ZxDisplay::DISPLAY_HEIGHT, QImage::Format_RGB32);
};

#endif // SCREEN_H
