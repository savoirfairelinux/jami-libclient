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
#include "newaccountmodel.h"

// Dbus
#include "dbus/configurationmanager.h"

// Models and database
#include "newcallmodel.h"
#include "contactmodel.h"
#include "conversationmodel.h"

namespace lrc
{

NewAccountModel::NewAccountModel(const Database& database)
: QObject()
, database_(database)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds) {
        QMap<QString, QString> details = ConfigurationManager::instance().getAccountDetails(id);

        auto info = std::make_shared<account::Info>();
        info->accountModel = std::shared_ptr<NewAccountModel>(this);
        info->id = id.toStdString();
        info->type = details["Account.type"] == "RING" ? account::Type::RING : account::Type::SIP;
        info->callModel = std::shared_ptr<NewCallModel>(new NewCallModel(*this, *info));
        info->contactModel = std::shared_ptr<ContactModel>(new ContactModel(*this, database, *info));
        info->conversationModel = std::shared_ptr<ConversationModel>(new ConversationModel(*this, database, *info));

        accounts_[id.toStdString()] = info;
    }
}

NewAccountModel::~NewAccountModel()
{

}

std::shared_ptr<account::Info>
NewAccountModel::getAccountInfo(const std::string& accountId)
{
    return accounts_[accountId];
}

void
NewAccountModel::setNewBuddySubscription(const std::string& accountId,
                                         const std::string& contactUri,
                                         bool status)
{
    auto& contactModel = getAccountInfo(accountId)->contactModel;
    contactModel->setContactPresent(contactUri, status);
}

void
NewAccountModel::slotContactAdded(const std::string& accountId,
                                  const std::string& contactUri,
                                  bool confirmed)
{
    auto& contactModel = getAccountInfo(accountId)->contactModel;
    contactModel->slotContactAdded(contactUri, confirmed);
}

void
NewAccountModel::slotContactRemoved(const std::string& accountId,
                                    const std::string& contactUri,
                                    bool banned)
{
    auto& contactModel = getAccountInfo(accountId)->contactModel;
    contactModel->slotContactRemoved(contactUri, banned);
}


} // namespace lrc
