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
#include "api/newaccountmodel.h"


// LRC
#include "database.h"
#include "api/newcallmodel.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"
#include "api/account.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class NewAccountModelPimpl
{
public:
    NewAccountModelPimpl(const NewAccountModel& linked, const Database& database);
    ~NewAccountModelPimpl();

    const NewAccountModel& linked;
    const Database& database;
    NewAccountModel::AccountInfoMap accounts;
};

NewAccountModel::NewAccountModel(const Database& database)
: QObject()
, pimpl_(std::make_unique<NewAccountModelPimpl>(*this, database))
{

}

NewAccountModel::~NewAccountModel()
{
}

const std::vector<std::string>
NewAccountModel::getAccountList() const
{
    std::vector<std::string> accountsId;

    for(auto const& accountInfo: pimpl_->accounts)
        accountsId.emplace_back(accountInfo.first);

    return std::move(accountsId);
}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId)
{
    return pimpl_->accounts[accountId];
}

NewAccountModelPimpl::NewAccountModelPimpl(const NewAccountModel& linked, const Database& database)
: linked(linked)
, database(database)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds) {
        QMap<QString, QString> details = ConfigurationManager::instance().getAccountDetails(id);

        account::Info owner;
        owner.accountModel = std::unique_ptr<const NewAccountModel>(&linked);
        owner.id = id.toStdString();
        owner.type = details["Account.type"] == "RING" ? account::Type::RING : account::Type::SIP;
        owner.callModel = std::make_unique<NewCallModel>(owner);
        owner.contactModel = std::make_unique<ContactModel>(owner, database);
        owner.conversationModel = std::make_unique<ConversationModel>(owner, database);

        accounts[id.toStdString()] = std::move(owner);
    }
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
