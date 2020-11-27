/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>
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

#pragma once

#include <interfaces/dbuserrorhandleri.h>

namespace Interfaces {

class DBusErrorHandler : public QObject, public DBusErrorHandlerI
{
    Q_OBJECT
public:
    DBusErrorHandler() {};
    ~DBusErrorHandler() {};

    Q_INVOKABLE void setActive(bool active);

    void connectionError(const QString& error) override;
    void invalidInterfaceError(const QString& error) override;

    void finishedHandlingError();

signals:
    void showDaemonReconnectPopup(bool visible);
    void daemonReconnectFailed();

private:
    void errorCallback();

    // Keeps track if we're in the process of handling an error already,
    // so that we don't keep displaying error dialogs;
    // we use an atomic in case the errors come from multiple threads
    std::atomic_bool handlingError {false};

    bool handlerActive_ {false};
};

} // namespace Interfaces
Q_DECLARE_METATYPE(Interfaces::DBusErrorHandler*)
