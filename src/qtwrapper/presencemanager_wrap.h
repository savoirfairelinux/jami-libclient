/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
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
#ifndef PRESENCEMANAGER_DBUS_INTERFACE_H_1416889973
#define PRESENCEMANAGER_DBUS_INTERFACE_H_1416889973

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include <sflphone.h>

#include "../dbus/metatypes.h"
#include "conversions_wrap.hpp"


/*
 * Proxy class for interface org.sflphone.SFLphone.PresenceManager
 */
class PresenceManagerInterface: public QObject
{
    Q_OBJECT
public:
    PresenceManagerInterface()
    {
        pres_ev_handlers = {
            .on_new_server_subscription_request = [this] (const std::string &buddyUri) { emit this->newServerSubscriptionRequest(QString(buddyUri.c_str())); },
            .on_server_error = [this] (const std::string &accountID, const std::string &error, const std::string &msg) { emit this->serverError(QString(accountID.c_str()), QString(error.c_str()), QString(msg.c_str())); },
            .on_new_buddy_notification = [this] (const std::string &accountID, const std::string &buddyUri, bool status, const std::string &lineStatus) { emit this->newBuddyNotification(QString(accountID.c_str()), QString(buddyUri.c_str()), status, QString(lineStatus.c_str())); },
            .on_subscription_state_change = [this] (const std::string &accountID, const std::string &buddyUri, bool state) { emit this->subscriptionStateChanged(QString(accountID.c_str()), QString(buddyUri.c_str()), state); }
        };
    }

    ~PresenceManagerInterface() {}

    sflph_pres_ev_handlers pres_ev_handlers;

public Q_SLOTS: // METHODS
    void answerServerRequest(const QString &uri, bool flag)
    {
        sflph_pres_answer_server_request(uri.toStdString(), flag);
    }

    VectorMapStringString getSubscriptions(const QString &accountID)
    {
        VectorMapStringString temp;
        for (auto x : sflph_pres_get_subscriptions(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    void publish(const QString &accountID, bool status, const QString &note)
    {
        sflph_pres_publish(accountID.toStdString(), status, note.toStdString());
    }

    void setSubscriptions(const QString &accountID, const QStringList &uriList)
    {
        sflph_pres_set_subscriptions(accountID.toStdString(), convertStringList(uriList));
    }

    void subscribeBuddy(const QString &accountID, const QString &uri, bool flag)
    {
        sflph_pres_subscribe_buddy(accountID.toStdString(), uri.toStdString(), flag);
    }

Q_SIGNALS: // SIGNALS
    void newServerSubscriptionRequest(const QString &buddyUri);
    void serverError(const QString &accountID, const QString &error, const QString &msg);
    void newBuddyNotification(const QString &accountID, const QString &buddyUri, bool status, const QString &lineStatus);
    void subscriptionStateChanged(const QString &accountID, const QString &buddyUri, bool state);
};

namespace org {
  namespace sflphone {
    namespace SFLphone {
      typedef ::PresenceManagerInterface PresenceManager;
    }
  }
}
#endif
