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
#include "newaccountmodel.h"
#include "database.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"
#include "data/message.h"

namespace lrc
{

Lrc::Lrc()
: QObject(nullptr)
{
    // create the database manager
    if (not database_) {
        Database* ptr = new Database();
        database_ = std::unique_ptr<Database>(ptr);
    }

    // create the account model
    if (not accountModel_) {
        NewAccountModel* ptr = new NewAccountModel(*database_.get());
        accountModel_ = std::unique_ptr<NewAccountModel>(ptr);
    }
    // Get signals from daemon
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingAccountMessage,
            this,
            &Lrc::newAccountMessage);

    connect(&PresenceManager::instance(),
            SIGNAL(newBuddyNotification(QString,QString,bool,QString)),
            this,
            SLOT(slotNewBuddySubscription(QString,QString,bool,QString)));

}

Lrc::~Lrc()
{

}

void
Lrc::newAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads)
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
    auto contactModel = accountModel_->getAccountInfo(accountId.toStdString()).contactModel;
    contactModel->setContactPresent(uri.toStdString(), status);
}

} // namespace lrc
