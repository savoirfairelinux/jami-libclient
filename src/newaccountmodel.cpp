/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include "api/behaviorcontroller.h"
#include "authority/databasehelper.h"
#include "callbackshandler.h"
#include "database.h"

#include "accountmodel.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class NewAccountModelPimpl: public QObject
{
    Q_OBJECT
public:
    NewAccountModelPimpl(NewAccountModel& linked,
                         Database& database,
                         const CallbacksHandler& callbackHandler,
                         const BehaviorController& behaviorController);
    ~NewAccountModelPimpl();

    NewAccountModel& linked;
    const CallbacksHandler& callbacksHandler;
    Database& database;
    NewAccountModel::AccountInfoMap accounts;
    const BehaviorController& behaviorController;

    /**
     * Add the profile information from an account to the db then add it to accounts.
     * @param accountId
     * @note this method get details for an account from the daemon.
     */
    void addToAccounts(const std::string& accountId);

public Q_SLOTS:
    /**
     * Emit accountStatusChanged.
     * @param accountId
     * @param status
     */
    void slotAccountStatusChanged(const std::string& accountID, const api::account::Status status);
    /**
     * Emit accountRemoved.
     * @param account
     */
    void slotAccountRemoved(Account* account);
};

NewAccountModel::NewAccountModel(Database& database,
                                 const CallbacksHandler& callbacksHandler,
                                 const BehaviorController& behaviorController)
: QObject()
, pimpl_(std::make_unique<NewAccountModelPimpl>(*this, database, callbacksHandler, behaviorController))
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
                                           const CallbacksHandler& callbacksHandler,
                                           const BehaviorController& behaviorController)
: linked(linked)
, behaviorController(behaviorController)
, callbacksHandler(callbacksHandler)
, database(database)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds)
        addToAccounts(id.toStdString());

    connect(&callbacksHandler, &CallbacksHandler::accountStatusChanged, this, &NewAccountModelPimpl::slotAccountStatusChanged);

    // NOTE: because we still use the legacy LRC for configuration, we are still using old signals
    connect(&AccountModel::instance(), &AccountModel::accountRemoved, this,  &NewAccountModelPimpl::slotAccountRemoved);
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

void
NewAccountModelPimpl::slotAccountStatusChanged(const std::string& accountID, const api::account::Status status)
{
    auto accountInfo = accounts.find(accountID);
    if (status == api::account::Status::REGISTERED && accountInfo == accounts.end()) {
        // Update account
        // NOTE we don't connect to newAccountAdded from AccountModel
        // because the account is not ready.
        accounts.erase(accountID);
        addToAccounts(accountID);
        emit linked.accountAdded(accountID);
    } else if (accountInfo != accounts.end()) {
        if (status == api::account::Status::REGISTERED) {
            accounts[accountID].enabled = true;
        } else if (status == api::account::Status::UNREGISTERED) {
            accounts[accountID].enabled = false;
        }
        accountInfo->second.status = status;
        emit linked.accountStatusChanged(accountID);
    }
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
    owner.profileInfo.type = details["Account.type"] == "RING" ? profile::Type::RING : profile::Type::SIP;
    owner.profileInfo.alias = details["Account.alias"].toStdString();
    owner.registeredName = owner.profileInfo.type == profile::Type::RING ?
                                   volatileDetails["Account.registredName"].toStdString() : owner.profileInfo.alias;
    owner.profileInfo.uri = (owner.profileInfo.type == profile::Type::RING and details["Account.username"].contains("ring:")) ?
                        details["Account.username"].toStdString().substr(std::string("ring:").size())
                        : details["Account.username"].toStdString();
    // Add profile into database
    auto accountProfileId = authority::database::getOrInsertProfile(database, owner.profileInfo.uri,
                                                                    owner.profileInfo.alias, "",
                                                                    details["Account.type"].toStdString());
    // Retrieve avatar from database
    auto accountInfo = authority::database::getAvatarForProfileId(database, accountProfileId);
    owner.profileInfo.avatar = accountInfo.profileInfo.avatar;
    // Init models for this account
    owner.callModel = std::make_unique<NewCallModel>(owner, callbacksHandler);
    owner.contactModel = std::make_unique<ContactModel>(owner, database, callbacksHandler);
    owner.conversationModel = std::make_unique<ConversationModel>(owner, database, callbacksHandler, behaviorController);
    owner.accountModel = &linked;
}

void
NewAccountModelPimpl::slotAccountRemoved(Account* account)
{
    auto accountId = account->id().toStdString();
    authority::database::removeAccount(database, accounts[accountId].profileInfo.uri);
    accounts.erase(accountId);
    emit linked.accountRemoved(accountId);
}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
#include "newaccountmodel.moc"
