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
    Database& database;
    NewAccountModel::AccountInfoMap accounts;

    void addAcountProfileInDb(const account::Info& info);

public Q_SLOTS:
    void slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId);
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
        const MapStringString volatileDetails = ConfigurationManager::instance().getVolatileAccountDetails(id);

        auto& item = *(accounts.emplace(id.toStdString(), account::Info()).first);
        auto& owner = item.second;
        owner.id = id.toStdString();
        owner.profile.uri = details["Account.username"].toStdString();
        // TODO get avatar;
        owner.profile.registeredName = volatileDetails["Account.registredName"].toStdString();
        owner.profile.alias = details["Account.alias"].toStdString();
        owner.profile.type = details["Account.type"] == "RING" ? contact::Type::RING : contact::Type::SIP;
        owner.callModel = std::make_unique<NewCallModel>(owner, callbacksHandler);
        owner.contactModel = std::make_unique<ContactModel>(owner, database, callbacksHandler);
        owner.conversationModel = std::make_unique<ConversationModel>(owner, database, callbacksHandler);
        owner.accountModel = &linked;
        addAcountProfileInDb(owner);
    }

    connect(&callbacksHandler, &CallbacksHandler::incomingCall, this, &NewAccountModelPimpl::slotIncomingCall);
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
                                        {{":uri", info.profile.uri}});
    if (returnFromDb.payloads.empty()) {
        // Profile is not in db, add it.
        auto type = info.profile.type == contact::Type::RING ? "RING" : "SIP";
        database.insertInto("profiles",
                             {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"},
                              {":type", "type"}, {":status", "status"}},
                             {{":uri", info.profile.uri}, {":alias", info.profile.alias}, {":photo", ""},
                              {":type", type}, {":status", ""}});
    }
}

void
NewAccountModelPimpl::slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId)
{
    emit linked.incomingCall(accountId, fromId);
}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
