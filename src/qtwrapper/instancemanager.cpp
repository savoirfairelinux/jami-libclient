/****************************************************************************
 *   Copyright (C) 2009-2018 Savoir-faire Linux                          *
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

InstanceManagerInterface::InstanceManagerInterface() : m_pTimer(nullptr)
{
   using namespace std::placeholders;

   using std::bind;
   using DRing::exportable_callback;
   using DRing::CallSignal;
   using DRing::ConfigurationSignal;
   using DRing::PresenceSignal;
   using DRing::DataTransferSignal;

#ifdef ENABLE_VIDEO
   using DRing::VideoSignal;
#endif

   m_pTimer = new QTimer(this);
   m_pTimer->setInterval(50);
#ifdef Q_OS_WIN
   connect(m_pTimer,SIGNAL(timeout()),this,SLOT(pollEvents()));
#else
   connect(m_pTimer,&QTimer::timeout,this,&InstanceManagerInterface::pollEvents);
#endif
   m_pTimer->start();

#ifndef MUTE_DRING
   ringFlags |= DRing::DRING_FLAG_DEBUG;
   ringFlags |= DRing::DRING_FLAG_CONSOLE_LOG;
#endif

   DRing::init(static_cast<DRing::InitFlag>(ringFlags));

   registerCallHandlers(CallManager::instance().callHandlers);
   registerConfHandlers(ConfigurationManager::instance().confHandlers);
   registerPresHandlers(PresenceManager::instance().presHandlers);
   registerDataXferHandlers(ConfigurationManager::instance().dataXferHandlers);
#ifdef ENABLE_VIDEO
   registerVideoHandlers(VideoManager::instance().videoHandlers);
#endif

   if (!DRing::start())
      printf("Error initializing daemon\n");
   else
      printf("Daemon is running\n");
}

InstanceManagerInterface::~InstanceManagerInterface()
{

}

void InstanceManagerInterface::pollEvents()
{
   DRing::pollEvents();
}

bool InstanceManagerInterface::isConnected()
{
   return true;
}
