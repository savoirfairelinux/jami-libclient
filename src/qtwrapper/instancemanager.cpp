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
#ifdef Q_OS_WIN
   connect(m_pTimer,SIGNAL(timeout()),this,SLOT(pollEvents()));
#else
   connect(m_pTimer,&QTimer::timeout,this,&InstanceInterface::pollEvents);
#endif
   m_pTimer->start();
   ringFlags |= DRing::DRING_FLAG_DEBUG;
   ringFlags |= DRing::DRING_FLAG_CONSOLE_LOG;

   DRing::init(static_cast<DRing::InitFlag>(ringFlags));

   registerCallHandlers(DBus::CallManager::instance().callHandlers);
   registerConfHandlers(DBus::ConfigurationManager::instance().confHandlers);
   registerPresHandlers(DBus::PresenceManager::instance().presHandlers);
#ifdef ENABLE_VIDEO
   registerVideoHandlers(DBus::VideoManager::instance().videoHandlers);
#endif

   if (!DRing::start())
      printf("Error initializing daemon\n");
   else
      printf("Daemon is running\n");
}

InstanceInterface::~InstanceInterface()
{

}

void InstanceInterface::pollEvents()
{
   DRing::pollEvents();
}

bool InstanceInterface::isConnected()
{
   return true;
}
