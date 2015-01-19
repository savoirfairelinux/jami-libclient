/****************************************************************************
 *   Copyright (C) 2009-2014 by Savoir-Faire Linux                          *
 *   Authors : Alexandre Lision alexandre.lision@savoirfairelinux.com       *
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

#include "instancemanager_wrap.h"
#include "callmanager.h"
#include "presencemanager.h"
#include "configurationmanager.h"
#ifdef ENABLE_VIDEO
 #include "videomanager.h"
#endif //ENABLE_VIDEO

static int ringFlags = 0;

InstanceInterface::InstanceInterface() : m_pTimer(nullptr)
{
   m_pTimer = new QTimer(this);
   m_pTimer->setInterval(50);
   connect(m_pTimer,SIGNAL(timeout()),this,SLOT(pollEvents()));
   ringFlags |= RING_FLAG_DEBUG;
   ringFlags |= RING_FLAG_CONSOLE_LOG;

   evHandlers = {
       .call_ev_handlers = DBus::CallManager::instance().call_ev_handlers,
       .config_ev_handlers = DBus::ConfigurationManager::instance().config_ev_handlers,
       .pres_ev_handlers = DBus::PresenceManager::instance().pres_ev_handlers,
   #ifdef ENABLE_VIDEO
      .video_ev_handler = DBus::VideoManager::instance().video_ev_handlers
   #endif /* ENABLE_VIDEO */
   };

   ring_init(&evHandlers, static_cast<ring_init_flag>(ringFlags));

   printf("INITIATED DAEMON\n");
}

InstanceInterface::~InstanceInterface()
{

}

void pollEvents()
{
   ring_poll_events();
}

bool InstanceInterface::isConnected()
{
   return true;
}


