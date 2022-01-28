/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

#include <QObject>

#ifdef Q_OS_WIN
class ConnectivityMonitor final : public QObject
{
    Q_OBJECT
public:
    explicit ConnectivityMonitor(QObject* parent = nullptr);
    ~ConnectivityMonitor();

    bool isOnline();

Q_SIGNALS:
    void connectivityChanged();

private:
    void destroy();

    struct INetworkListManager* pNetworkListManager_;
    struct IConnectionPointContainer* pCPContainer_;
    struct IConnectionPoint* pConnectPoint_;
    class NetworkEventHandler* netEventHandler_;
    unsigned long cookie_;
};

#else
// TODO: platform implementations should be in the daemon.

class ConnectivityMonitor final : public QObject
{
    Q_OBJECT
public:
    explicit ConnectivityMonitor(QObject* parent = nullptr);
    ~ConnectivityMonitor();

    bool isOnline();

Q_SIGNALS:
    void connectivityChanged();
};
#endif // Q_OS_WIN
