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
#include "api/newcallmodel.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"
#include "api/account.h"
#include "callbackshandler.h"
#include "database.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class NewAccountModelPimpl
{
public:
    NewAccountModelPimpl(NewAccountModel& linked,
                         Database& database,
                         const CallbacksHandler& callbackHandler);
    ~NewAccountModelPimpl();

    NewAccountModel& linked;
    const Database& database;
    NewAccountModel::AccountInfoMap accounts;
};

NewAccountModel::NewAccountModel(Database& database, const CallbacksHandler& callbacksHandler)
: QObject()
, pimpl_(std::make_unique<NewAccountModelPimpl>(*this, database, callbacksHandler))
{

}

NewAccountModel::~NewAccountModel()
{
}

std::vector<std::string>
NewAccountModel::getAccountList() const
{
    std::vector<std::string> accountsId;

    for(auto const& accountInfo: pimpl_->accounts)
        accountsId.emplace_back(accountInfo.first);

    return accountsId;
}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId) const
{
    auto accountInfo = pimpl_->accounts.find(accountId);
    if (accountInfo == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::getAccountInfo, can't find " + accountId);

    return accountInfo->second;
}

NewAccountModelPimpl::NewAccountModelPimpl(NewAccountModel& linked,
                                           Database& database,
                                           const CallbacksHandler& callbacksHandler)
: linked(linked)
, database(database)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds) {
        QMap<QString, QString> details = ConfigurationManager::instance().getAccountDetails(id);

        auto& item = *(accounts.emplace(id.toStdString(), account::Info()).first);
        auto& owner = item.second;
        owner.id = id.toStdString();
        owner.type = details["Account.type"] == "RING" ? account::Type::RING : account::Type::SIP;
        owner.callModel = std::make_unique<NewCallModel>(owner);
        owner.contactModel = std::make_unique<ContactModel>(owner, database);
        owner.conversationModel = std::make_unique<ConversationModel>(owner, database);
        owner.accountModel = &linked;
    }
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
