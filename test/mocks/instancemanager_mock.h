/*
 *  Copyright (C) 2017-2018 Savoir-faire Linux Inc.
 *
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
#pragma once

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTimer>

#include "dring.h"
#include "typedefs.h"
#include "qtwrapper/conversions_wrap.hpp"

/**
 * Proxy class for interface org.ring.Ring.Instance
 */
class InstanceManagerInterface: public QObject
{
    Q_OBJECT
public:
    InstanceManagerInterface() {}
    ~InstanceManagerInterface() {}

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

    bool isConnected() {return false;}

    void pollEvents() {}

private:

Q_SIGNALS: // SIGNALS
    void started();
};

namespace cx {
  namespace Ring {
    namespace Ring {
      typedef ::InstanceManagerInterface Instance;
    }
  }
}
