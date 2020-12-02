/*
 *  Copyright (C) 2020 Savoir-faire Linux Inc.
 *
 *  Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>
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
#include "conversationmanager.h"

#include "../globalinstances.h"
#include "../interfaces/dbuserrorhandleri.h"

ConversationManagerInterface&
ConversationManager::instance()
{
#ifdef ENABLE_LIBWRAP
    static auto interface = new ConversationManagerInterface();
#else
    if (!dbus_metaTypeInit)
        registerCommTypes();

    static auto interface = new ConversationManagerInterface("cx.ring.Ring",
                                                             "/cx/ring/Ring/ConversationManager",
                                                             QDBusConnection::sessionBus());

    if (!interface->connection().isConnected()) {
        GlobalInstances::dBusErrorHandler().connectionError(
            "Error : dring not connected. Service " + interface->service()
            + " not connected. From conversation manager interface.");
    }
    if (!interface->isValid()) {
        GlobalInstances::dBusErrorHandler().invalidInterfaceError(
            "Error : dring is not available, make sure it is running");
    }
#endif
    return *interface;
}
