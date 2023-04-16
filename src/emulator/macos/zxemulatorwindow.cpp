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
#include "zxemulatorscreen.h"
#include "zxemulatorwindow.h"
#include <QtWidgets>
#include <QTimer>
#include <utility>
#include <common/clock.h>
#include <common/hardware/zxhardwaremodel48k.h>
#include <zx48k_rom.h>
#include <common/Z80emu.h>


ZxEmulatorWindow::ZxEmulatorWindow(QString programFile) : m_programFile(std::move(programFile)) {

    qDebug() << "Program to run: " << ((m_programFile != nullptr) ? m_programFile : "NONE");

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

    Clock::getInstance().setSpectrumModel(m_model);
    m_z80emu->initialise(zx48k_rom, zx48k_rom_len);

    if (m_programFile != nullptr) {
        qDebug() << "Loading program snapshot:" << m_programFile;
        QFile programFile(m_programFile);
            if (programFile.open(QIODevice::ReadOnly)) {
                QByteArray data = programFile.readAll();
                m_z80emu->loadSnapshot(reinterpret_cast<uint8_t *>(data.data()), static_cast<int32_t>(data.size()));
            } else {
                qDebug() << "Unable to find program file" << m_programFile;
            }
    }

    m_timer->setTimerType(Qt::PreciseTimer);
    QObject::connect(m_timer, &QTimer::timeout, this, &ZxEmulatorWindow::execute);

    // Call timer every 20 milliseconds to simulate a 50Hz refresh rate
    m_timer->start(20);
}


void ZxEmulatorWindow::execute() {

    m_z80emu->execute(m_model->tStatesPerScreenFrame());
    Clock::getInstance().endFrame();
    m_screen->repaint();
}


void ZxEmulatorWindow::keyPressEvent(QKeyEvent *event) {

    qDebug() << "ZxEmulatorWindow::keyPressEvent" << event->text();
}
