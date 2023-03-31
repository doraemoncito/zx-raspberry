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
#include "zxemulatorscreen.h"
#include "zxemulatorwindow.h"
#include <QtWidgets>
#include <QTimer>
#include <common/clock.h>
#include <common/hardware/zxhardwaremodel48k.h>
#include <zx48k_rom.h>
#include <common/Z80emu.h>
#include <common/aquaplane_sna.h>
//#include <common/automania_sna.h>


ZxEmulatorWindow::ZxEmulatorWindow() {

    m_pZxDisplay = new ZxDisplay();
    m_z80emu = new Z80emu(m_pZxDisplay);
    m_timer = new QTimer(this);
    m_model = new ZxHardwareModel48k();
    m_screen = new ZxEmulatorScreen(m_z80emu, m_pZxDisplay, this);
    auto *mainLayout = new QGridLayout;

    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->addWidget(m_screen, 0, 0, 1, 1);
    setLayout(mainLayout);

    setWindowTitle(tr("ZX Raspberry - a bare metal ZX Spectrum Emulator for Raspberry Pi"));
    QTimer::singleShot(0, this, SLOT(initialise()));
}


ZxEmulatorWindow::~ZxEmulatorWindow() {

    delete m_screen;
    delete m_z80emu;
    delete m_model;
}


void ZxEmulatorWindow::initialise() {

    m_z80emu->initialise(zx48k_rom, zx48k_rom_len);
    qDebug() << "Loading game in snapshot format";
    m_z80emu->loadSnapshot(aquaplane_sna);
//    m_z80emu->loadSnapshot(automania_sna);

    Clock::getInstance().setSpectrumModel(m_model);

    m_timer->setTimerType(Qt::PreciseTimer);
    QObject::connect(m_timer, &QTimer::timeout, this, &ZxEmulatorWindow::execute);

    // Call timer every 20 milliseconds to simulate a 50Hz refresh rate
    m_timer->start(20);
}


void ZxEmulatorWindow::execute() {

    m_z80emu->execute(m_model->tStatesPerScreenFrame());
    m_screen->repaint();
    Clock::getInstance().endFrame();
}


void ZxEmulatorWindow::keyPressEvent(QKeyEvent *event) {

    qDebug() << "ZxEmulatorWindow::keyPressEvent";
}
