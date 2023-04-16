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
#ifndef CIRCLE_LOGGER_H
#define CIRCLE_LOGGER_H

#include <cstdarg>
#include <QDebug>

enum TLogSeverity
{
    LogPanic,	// Halt the system after processing this message
    LogError,	// Severe error in this component, system may continue to work
    LogWarning,	// Non-severe problem, component continues to work
    LogNotice,	// Informative message, which is interesting for the system user
    LogDebug	// Message, which is only interesting for debugging this component
};

class CLogger {

private:
    static CLogger *instance;

public:
    CLogger() = default;
    static CLogger *Get() { return instance; };
    void Write(const char *pSource, TLogSeverity /* Severity */, const char *pMessage, ...) {
        char message[256];
        va_list argptr;
        va_start(argptr, pMessage);
        vsprintf(message, pMessage, argptr);
        va_end(argptr);
        qDebug() << pSource << message;
    };

};

#endif // CIRCLE_LOGGER_H
