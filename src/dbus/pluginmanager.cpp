/****************************************************************************
 *    Copyright (C) 2013-2022 Savoir-faire Linux Inc.                       *
 *   Author : Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com> *
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
#include "pluginmanager.h"

#include "../globalinstances.h"
#include "../interfaces/dbuserrorhandleri.h"

PluginManagerInterface&
PluginManager::instance()
{
#ifdef ENABLE_LIBWRAP
    static auto interface = new PluginManagerInterface();
#else
    if (!dbus_metaTypeInit)
        registerCommTypes();
    static auto interface = new PluginManagerInterface("cx.ring.Ring",
                                                       "/cx/ring/Ring/PluginManagerInterface",
                                                       QDBusConnection::sessionBus());

    if (!interface->connection().isConnected()) {
        GlobalInstances::dBusErrorHandler().connectionError(
            "Error : jamid not connected. Service " + interface->service()
            + " not connected. From presence interface.");
    }
    if (!interface->isValid()) {
        GlobalInstances::dBusErrorHandler().invalidInterfaceError(
            "Error : jamid is not available, make sure it is running");
    }
#endif
    return *interface;
}
