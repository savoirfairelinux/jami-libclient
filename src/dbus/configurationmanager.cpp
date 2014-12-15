/****************************************************************************
 *   Copyright (C) 2009-2014 by Savoir-Faire Linux                          *
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
#include "configurationmanager.h"
#include "callbacks.h"

static int sflphFlags = 0;

ConfigurationManagerInterface* DBus::ConfigurationManager::interface = nullptr;

ConfigurationManagerInterface& DBus::ConfigurationManager::instance()
{
#ifdef ENABLE_LIBWRAP
   if (!interface) {
        interface = new ConfigurationManagerInterface();
        initDaemon();
    }
#else
   if (!dbus_metaTypeInit) registerCommTypes();
   if (!interface)
      interface = new ConfigurationManagerInterface("cx.ring.Ring", "/cx/ring/Ring/ConfigurationManager", QDBusConnection::sessionBus());
   if(!interface->connection().isConnected()) {
      qDebug() << "Error : dring not connected. Service " << interface->service() << " not connected. From configuration manager interface.";
      throw "Error : dring not connected. Service " + interface->service() + " not connected. From configuration manager interface.";
   }
   if (!interface->isValid())
      throw "DRing daemon not available, be sure it running";
#endif
   return *interface;
}

void DBus::ConfigurationManager::initDaemon()
{
    // TODO: Discuss with Elv where to put this.
    sflphFlags |= SFLPH_FLAG_DEBUG;
    sflphFlags |= SFLPH_FLAG_CONSOLE_LOG;

    interface->evHandlers = {
        .call_ev_handlers = CallManager::instance().call_ev_handlers,
        .config_ev_handlers = interface->config_ev_handlers,
        .pres_ev_handlers = PresenceManager::instance().pres_ev_handlers,
    #ifdef ENABLE_VIDEO
        .video_ev_handler = VideoManager::instance().video_ev_handlers
    #endif /* ENABLE_VIDEO */
    };

    interface->evHandlers.call_ev_handlers.on_state_change("tom", "pot");

    sflph_init(&interface->evHandlers, static_cast<sflph_init_flag>(sflphFlags));

    printf("INITIATED DAEMON\n");
}
