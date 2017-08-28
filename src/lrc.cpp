/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include "lrc.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"

// Models and database
#include "database.h"
#include "newaccountmodel.h"
#include "contactmodel.h"

namespace lrc
{

Lrc::Lrc()
: QObject()
{
    // create the database manager
    database_ = std::unique_ptr<Database>(new Database());

    // create the account model
    accountModel_ = std::unique_ptr<NewAccountModel>(new NewAccountModel(*database_.get()));

    // Get signals from daemon
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingAccountMessage,
            this,
            &Lrc::slotNewAccountMessage);

    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::newBuddyNotification,
            this,
            &Lrc::slotNewBuddySubscription);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactAdded,
            this,
            &Lrc::slotContactAdded);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactRemoved,
            this,
            &Lrc::slotContactRemoved);
}

Lrc::~Lrc()
{

}

void
Lrc::slotNewAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads)
{
    message::Info msg;
    msg.uid = from.toStdString();
    msg.body = payloads["text/plain"].toStdString();
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::READ;
    database_->addMessage(accountId.toStdString(), msg);
}

void
Lrc::slotNewBuddySubscription(const QString& accountId, const QString& uri, bool status, const QString& message)
{
    Q_UNUSED(message)
    accountModel_->setNewBuddySubscription(accountId.toStdString(),
                                           contactUri.toStdString(),
                                           status);
}

void
Lrc::slotContactAdded(const QString& accountId,
                      const QString& contactUri,
                      bool confirmed)
{
    Q_UNUSED(confirmed)
    accountModel_->slotContactAdded(accountId.toStdString(),
                                    contactUri.toStdString(),
                                    confirmed);
}

void
Lrc::slotContactRemoved(const QString& accountId,
                        const QString& contactUri,
                        bool banned)
{
    Q_UNUSED(banned)
    accountModel_->slotContactRemoved(accountId.toStdString(),
                                      contactUri.toStdString(),
                                      banned);
}

} // namespace lrc
