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

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

#include "typedefs.h"
#include <presencemanager_interface.h>
#include "qtwrapper/conversions_wrap.hpp"


/*
 * Proxy class for interface org.ring.Ring.PresenceManager
 */
class PresenceManagerInterface: public QObject
{
    Q_OBJECT
public:


    PresenceManagerInterface()
    {
    }

    ~PresenceManagerInterface() {}

    void emitNewBuddyNotification(const QString &accountID, const QString &buddyUri, bool status, const QString &lineStatus)
    {
        emit newBuddyNotification(accountID, buddyUri, status, lineStatus);
    }

public Q_SLOTS: // METHODS
    void answerServerRequest(const QString &uri, bool flag)
    {
        Q_UNUSED(uri)
        Q_UNUSED(flag)
    }

    VectorMapStringString getSubscriptions(const QString &accountID)
    {
        Q_UNUSED(accountID)
        VectorMapStringString temp;
        return temp;
    }

    void publish(const QString &accountID, bool status, const QString &note)
    {
        Q_UNUSED(accountID)
        Q_UNUSED(status)
        Q_UNUSED(note)
    }

    void setSubscriptions(const QString &accountID, const QStringList &uriList)
    {
        Q_UNUSED(accountID)
        Q_UNUSED(uriList)
    }

    void subscribeBuddy(const QString &accountID, const QString &uri, bool flag)
    {
        Q_UNUSED(accountID)
        Q_UNUSED(uri)
        Q_UNUSED(flag)
    }

Q_SIGNALS: // SIGNALS
    void newServerSubscriptionRequest(const QString &buddyUri);
    void serverError(const QString &accountID, const QString &error, const QString &msg);
    void newBuddyNotification(const QString &accountID, const QString &buddyUri, bool status, const QString &lineStatus);
    void subscriptionStateChanged(const QString &accountID, const QString &buddyUri, bool state);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::PresenceManagerInterface PresenceManager;
    }
  }
}
