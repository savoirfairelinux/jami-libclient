/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "presencemanager.h"

#include "../globalinstances.h"
#include "../interfaces/dbuserrorhandleri.h"

PresenceManagerInterface& PresenceManager::instance()
{
#ifdef ENABLE_LIBWRAP
    static auto interface = new PresenceManagerInterface();
#else
    if (!dbus_metaTypeInit) registerCommTypes();
    static auto interface = new PresenceManagerInterface("cx.ring.Ring",
                                                        "/cx/ring/Ring/PresenceManager",
                                                        QDBusConnection::sessionBus());

    if (!interface->connection().isConnected()) {
        GlobalInstances::dBusErrorHandler().connectionError(
            "Error : dring not connected. Service " + interface->service() + " not connected. From presence interface."
        );
    }
    if (!interface->isValid()) {
        GlobalInstances::dBusErrorHandler().invalidInterfaceError(
            "Error : dring is not available, make sure it is running"
        );
    }
#endif
    return *interface;
}
