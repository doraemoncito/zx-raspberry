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
#include <QApplication>
#include <QCommandLineParser>
#include "zxemulatorwindow.h"
#include "common/Z80emu.h"


int main(int argc, char *argv[]) {

    QApplication application(argc, argv);
    QApplication::setApplicationName("ZX Raspberry");
    QApplication::setApplicationVersion("0.0.1");

    // https://doc.qt.io/qt-6/qcommandlineparser.html
    QCommandLineParser parser;
    parser.setApplicationDescription("ZX Raspberry - A bare metal ZX Spectrum emulator");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("program-file", QCoreApplication::translate("program-file", "Program file to run."));
    parser.process(application);
    const QStringList args = parser.positionalArguments();
    // Position 0, if present, should contain the full path to the ZX Spectrum program to run
    const QString &programFile = args.value(0);

    auto *emulatorWindow = new ZxEmulatorWindow(programFile);
    emulatorWindow->show();

    return QApplication::exec();
}
