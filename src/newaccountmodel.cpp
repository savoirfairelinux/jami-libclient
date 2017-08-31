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

        account::Info info;
        info.accountModel = std::unique_ptr<NewAccountModel>(this);
        info.id = id.toStdString();
        info.type = details["Account.type"] == "RING" ? account::Type::RING : account::Type::SIP;
        info.callModel = std::unique_ptr<NewCallModel>(new NewCallModel(*this, info));
        info.contactModel = std::unique_ptr<ContactModel>(new ContactModel(*this, database, info));
        info.conversationModel = std::unique_ptr<ConversationModel>(new ConversationModel(*this, database, info));

        accounts_[id.toStdString()] = std::move(info);
    }
}

NewAccountModel::~NewAccountModel()
{

}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId)
{
    return accounts_[accountId];
}

} // namespace lrc
