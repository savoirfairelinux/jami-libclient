/*!
 * Copyright (C) 2020-2022 Savoir-faire Linux Inc.
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbuserrorhandler.h"

#include "api/lrc.h"
#include "globalinstances.h"

#include <QTimer>

namespace Interfaces {

void
DBusErrorHandler::errorCallback()
{
    qDebug() << "Dring has possibly crashed, "
                "or has been killed... will wait 2.5 seconds and try to reconnect";

    Q_EMIT showDaemonReconnectPopup(true);

    QTimer::singleShot(2500, [this]() {
        if ((!lrc::api::Lrc::isConnected()) || (!lrc::api::Lrc::dbusIsValid())) {
            qDebug() << "Could not reconnect to the daemon";
            Q_EMIT daemonReconnectFailed();
        } else {
            static_cast<DBusErrorHandler&>(GlobalInstances::dBusErrorHandler())
                .finishedHandlingError();
        }
    });
}

void
DBusErrorHandler::setActive(bool active)
{
    handlerActive_ = active;

    if (active) {
        if ((!lrc::api::Lrc::isConnected()) || (!lrc::api::Lrc::dbusIsValid()))
            connectionError(QString());
    }
}

void
DBusErrorHandler::connectionError(const QString& error)
{
    qDebug() << error;

    if (!handlerActive_)
        return;

    if (!handlingError) {
        handlingError = true;
        errorCallback();
    }
}

void
DBusErrorHandler::invalidInterfaceError(const QString& error)
{
    qDebug() << error;

    if (!handlerActive_)
        return;

    if (!handlingError) {
        handlingError = true;
        errorCallback();
    }
}

void
DBusErrorHandler::finishedHandlingError()
{
    handlingError = false;
    Q_EMIT showDaemonReconnectPopup(false);
}

} // namespace Interfaces
