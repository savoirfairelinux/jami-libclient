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
#ifndef PRESENCEMANAGER_DBUS_INTERFACE_H
#define PRESENCEMANAGER_DBUS_INTERFACE_H

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
#include "conversions_wrap.hpp"


/*
 * Proxy class for interface org.ring.Ring.PresenceManager
 */
class PresenceManagerInterface: public QObject
{
    Q_OBJECT
public:

    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> presHandlers;

    PresenceManagerInterface()
    {
        using DRing::exportable_callback;
        using DRing::PresenceSignal;

        presHandlers = {
            exportable_callback<PresenceSignal::NewServerSubscriptionRequest>(
                [this] (const std::string &buddyUri) {
                       QTimer::singleShot(0, [this,buddyUri] {
                             Q_EMIT this->newServerSubscriptionRequest(QString(buddyUri.c_str()));
                       });
            }),
            exportable_callback<PresenceSignal::ServerError>(
                [this] (const std::string &accountID, const std::string &error, const std::string &msg) {
                       QTimer::singleShot(0, [this,accountID, error, msg] {
                             Q_EMIT this->serverError(QString(accountID.c_str()), QString(error.c_str()), QString(msg.c_str()));
                       });
            }),
            exportable_callback<PresenceSignal::NewBuddyNotification>(
                [this] (const std::string &accountID, const std::string &buddyUri, bool status, const std::string &lineStatus) {
                       QTimer::singleShot(0, [this,accountID, buddyUri, status, lineStatus] {
                             Q_EMIT this->newBuddyNotification(QString(accountID.c_str()), QString(buddyUri.c_str()), status, QString(lineStatus.c_str()));
                       });
            }),
            exportable_callback<PresenceSignal::SubscriptionStateChanged>(
                [this] (const std::string &accountID, const std::string &buddyUri, bool state) {
                       QTimer::singleShot(0, [this,accountID, buddyUri, state] {
                             Q_EMIT this->subscriptionStateChanged(QString(accountID.c_str()), QString(buddyUri.c_str()), state);
                       });
            })
         };
    }

    ~PresenceManagerInterface() {}

public Q_SLOTS: // METHODS
    void answerServerRequest(const QString &uri, bool flag)
    {
        DRing::answerServerRequest(uri.toStdString(), flag);
    }

    VectorMapStringString getSubscriptions(const QString &accountID)
    {
        VectorMapStringString temp;
        for (auto x : DRing::getSubscriptions(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    void publish(const QString &accountID, bool status, const QString &note)
    {
        DRing::publish(accountID.toStdString(), status, note.toStdString());
    }

    void setSubscriptions(const QString &accountID, const QStringList &uriList)
    {
        DRing::setSubscriptions(accountID.toStdString(), convertStringList(uriList));
    }

    void subscribeBuddy(const QString &accountID, const QString &uri, bool flag)
    {
        DRing::subscribeBuddy(accountID.toStdString(), uri.toStdString(), flag);
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
#endif
