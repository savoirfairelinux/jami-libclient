/****************************************************************************
 *   Copyright (C) 2009-2017 Savoir-faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

#include "instancemanager.h"

#include <unistd.h>

#include "../globalinstances.h"
#include "../interfaces/dbuserrorhandleri.h"

InstanceManagerInterface& InstanceManager::instance()
{
#ifdef ENABLE_TEST
    static auto interface = new InstanceManagerInterface();
#else
 #ifdef ENABLE_LIBWRAP
    static auto interface = new InstanceManagerInterface();
 #else
    if (!dbus_metaTypeInit) registerCommTypes();

    static auto interface = new InstanceManagerInterface("cx.ring.Ring",
                                                "/cx/ring/Ring/Instance",
                                                QDBusConnection::sessionBus());

    if (!interface->connection().isConnected()) {
        GlobalInstances::dBusErrorHandler().connectionError(
            "Error : dring not connected. Service " + interface->service() + " not connected. From instance interface."
        );
    }
    static bool registered = false;
    if (!registered) {
        QDBusPendingReply<QString> reply = interface->Register(getpid(), "");
        registered = true;
        reply.waitForFinished();
    }

    /* we do not check if the interface isValid;
     * isValid() return 'false' if there was any error;
     * we expect there to be an error when we first launch the client and the daemon is not yet
     * running;
     * TODO: check if we get the expected error, or another, see:
     * http://doc.qt.io/qt-4.8/qdbuserror.html#ErrorType-enum
     */

 #endif
#endif
    return *interface;
}
