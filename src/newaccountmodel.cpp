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

class NewAccountModelPimpl: public QObject
{
public:
    NewAccountModelPimpl(NewAccountModel& linked,
                         Database& database,
                         const CallbacksHandler& callbackHandler);
    ~NewAccountModelPimpl();

    NewAccountModel& linked;
    const CallbacksHandler& callbacksHandler;
    Database& database;
    NewAccountModel::AccountInfoMap accounts;

    void addAcountProfileInDb(const account::Info& info);
    void addToAccounts(const std::string& accountId);

public Q_SLOTS:
    void slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId);
    void slotAccountStatusChanged(const std::string& accountID, const api::account::Status status);
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
, callbacksHandler(callbacksHandler)
, database(database)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds)
        addToAccounts(id.toStdString());

    connect(&callbacksHandler, &CallbacksHandler::incomingCall, this, &NewAccountModelPimpl::slotIncomingCall);
    connect(&callbacksHandler, &CallbacksHandler::accountStatusChanged, this, &NewAccountModelPimpl::slotAccountStatusChanged);
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

void
NewAccountModelPimpl::addAcountProfileInDb(const account::Info& info)
{
    auto returnFromDb = database.select("uri",
                                        "profiles",
                                        "uri=:uri",
                                        {{":uri", info.profileInfo.uri}});
    if (returnFromDb.payloads.empty()) {
        // Profile is not in db, add it.
        std::string type = info.profileInfo.type == profile::Type::RING ? "RING" : "SIP";
        database.insertInto("profiles",
                             {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"},
                              {":type", "type"}, {":status", "status"}},
                             {{":uri", info.profileInfo.uri}, {":alias", info.profileInfo.alias}, {":photo", ""},
                              {":type", type}, {":status", ""}});
    }
}

void
NewAccountModelPimpl::slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId)
{
    emit linked.incomingCall(accountId, fromId);
}

void
NewAccountModelPimpl::slotAccountStatusChanged(const std::string& accountID, const api::account::Status status)
{
    auto accountInfo = accounts.find(accountID);
        if (accountInfo == accounts.end())
            addToAccounts(accountID);

    accountInfo->second.status = status;

    emit linked.accountStatusChanged(accountID);
}

void
NewAccountModelPimpl::addToAccounts(const std::string& accountId)
{
    QMap<QString, QString> details = ConfigurationManager::instance().getAccountDetails(accountId.c_str());
    const MapStringString volatileDetails = ConfigurationManager::instance().getVolatileAccountDetails(accountId.c_str());

    // Init profile
    auto& item = *(accounts.emplace(accountId, account::Info()).first);
    auto& owner = item.second;
    owner.id = accountId;
    owner.enabled = details["Account.enable"] == QString("true");
    // TODO get avatar;
    owner.profileInfo.type = details["Account.type"] == "RING" ? profile::Type::RING : profile::Type::SIP;
    owner.profileInfo.alias = details["Account.alias"].toStdString();
    owner.registeredName = owner.profileInfo.type == profile::Type::RING ?
                                   volatileDetails["Account.registredName"].toStdString() : owner.profileInfo.alias;
    owner.profileInfo.uri = (owner.profileInfo.type == profile::Type::RING and details["Account.username"].contains("ring:")) ?
                        details["Account.username"].toStdString().substr(std::string("ring:").size())
                        : details["Account.username"].toStdString();
    // Add profile into database
    addAcountProfileInDb(owner);
    // Init models for this account
    owner.callModel = std::make_unique<NewCallModel>(owner, callbacksHandler);
    owner.contactModel = std::make_unique<ContactModel>(owner, database, callbacksHandler);
    owner.conversationModel = std::make_unique<ConversationModel>(owner, database, callbacksHandler);
    owner.accountModel = &linked;
}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
