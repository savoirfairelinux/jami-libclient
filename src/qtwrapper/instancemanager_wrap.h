/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>        *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#ifndef INSTANCE_DBUS_INTERFACE_H
#define INSTANCE_DBUS_INTERFACE_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTimer>

#include "dring.h"
#include "../typedefs.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface org.ring.Ring.Instance
 */
class InstanceInterface: public QObject
{
   Q_OBJECT
public:
   InstanceInterface();
   ~InstanceInterface();

// TODO: These are not present in dring.h

public Q_SLOTS: // METHODS
   void Register(int pid, const QString &name)
   {
      Q_UNUSED(pid ) //When directly linked, the PID is always the current process PID
      Q_UNUSED(name)
   }

   void Unregister(int pid)
   {
      Q_UNUSED(pid) //When directly linked, the PID is always the current process PID
   }

   bool isConnected();

   void pollEvents();

private:
   QTimer* m_pTimer;

Q_SIGNALS: // SIGNALS
   void started();
};

namespace cx {
  namespace Ring {
    namespace Ring {
      typedef ::InstanceInterface Instance;
    }
  }
}
#endif
