/*
 *  Copyright (C) 2017 Savoir-faire Linux Inc.
 *
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

#include "waitforsignalhelper.h"

#include <QTimer>

WaitForSignalHelper::WaitForSignalHelper(QObject& object, const char* signal)
: timeout_(false)
{
    connect(&object, signal, &eventLoop_, SLOT(quit()));
}

bool
WaitForSignalHelper::wait(unsigned int timeoutMs)
{
    QTimer timeoutHelper;
    if (timeoutMs != 0) {
        timeoutHelper.setInterval(timeoutMs);
        timeoutHelper.start();
        connect(&timeoutHelper, SIGNAL(timeout()), this, SLOT(timeout()));
    }
    timeout_ = false;
    eventLoop_.exec();
    return timeout_;
}

void
WaitForSignalHelper::timeout()
{
    timeout_ = true;
    eventLoop_.quit();
}
