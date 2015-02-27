/****************************************************************************
 *   Copyright (C) 2009-2014 by Savoir-Faire Linux                          *
 *   Authors : Alexandre Lision alexandre.lision@savoirfairelinux.com       *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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

void pollEvents();

InstanceInterface::InstanceInterface() : m_pTimer(nullptr)
{
    using namespace std::placeholders;

    using std::bind;
    using DRing::exportable_callback;
    using DRing::CallSignal;
    using DRing::ConfigurationSignal;
    using DRing::PresenceSignal;

    #ifdef ENABLE_VIDEO
    using DRing::VideoSignal;
    #endif

   m_pTimer = new QTimer(this);
   m_pTimer->setInterval(50);
   connect(m_pTimer,&QTimer::timeout,this,&pollEvents);
   m_pTimer->start();
   ringFlags |= DRing::DRING_FLAG_DEBUG;
   ringFlags |= DRing::DRING_FLAG_CONSOLE_LOG;

   const std::map<DRing::EventHandlerKey, std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>>> evHandlers = {
        { // Call event handlers
            DRing::EventHandlerKey::CALL, DBus::CallManager::instance().callHandlers
        },
        { // Configuration event handlers
            DRing::EventHandlerKey::CONFIG, DBus::ConfigurationManager::instance().confHandlers
        },
        { // Presence event handlers
            DRing::EventHandlerKey::PRESENCE, DBus::PresenceManager::instance().presHandlers
        }
#ifdef ENABLE_VIDEO
        ,{ // Video event handlers
            DRing::EventHandlerKey::VIDEO, DBus::VideoManager::instance().videoHandlers
        }
#endif
    };

    DRing::init(evHandlers, static_cast<DRing::InitFlag>(ringFlags));

    printf("INITIATED DAEMON\n");
}

InstanceInterface::~InstanceInterface()
{

}

void pollEvents()
{
    DRing::poll_events();
}

bool InstanceInterface::isConnected()
{
   return true;
}
