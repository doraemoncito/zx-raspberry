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
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>


class QTimer;
class ZxEmulatorScreen;
class Z80emu;
class ZxHardwareModel;
class ZxDisplay;

class ZxEmulatorWindow : public QWidget {
Q_OBJECT

public:
    ZxEmulatorWindow();
    ~ZxEmulatorWindow();

private slots:
    void initialise();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    ZxDisplay *m_pZxDisplay;
    ZxEmulatorScreen *m_screen;
    QTimer *m_timer;
    ZxHardwareModel *m_model;
    Z80emu *m_z80emu;

    void execute();

};

#endif // WINDOW_H
