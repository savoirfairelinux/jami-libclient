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

// Models and database
#include "callbackshandler.h"
#include "database.h"
#include "api/newcallmodel.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

namespace api
{

class NewAccountModelPimpl
{
public:
    NewAccountModelPimpl(const Database& database, const lrc::CallbacksHandler& callbackHandler);
    ~NewAccountModelPimpl();
    /**
     * Update the presence of a contact for an account
     * @param accountId
     * @param contactUri
     * @param status if the contact is present
     */
    void setNewBuddySubscription(const std::string& accountId, const std::string& contactUri, bool status);
    /**
     * Add a contact in the contact list of an account
     * @param accountId
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed);
    /**
     * Remove a contact from a contact list of an account
     * @param accountId
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned);

    const Database& database;
    const CallbacksHandler& callbackHandler;
    AccountInfoMap accounts;
};

NewAccountModel::NewAccountModel(const Database& database, const lrc::CallbacksHandler& callbackHandler)
: pimpl_(std::make_unique<NewAccountModelPimpl>(database, callbackHandler))
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds) {
        QMap<QString, QString> details = ConfigurationManager::instance().getAccountDetails(id);

        pimpl_->accounts.emplace(std::make_pair(id.toStdString(), account::Info()));
        auto& info = pimpl_->accounts[id.toStdString()];
        info.accountModel = std::unique_ptr<NewAccountModel>(this);
        info.id = id.toStdString();
        info.type = details["Account.type"] == "RING" ? account::Type::RING : account::Type::SIP;
        info.callModel = std::unique_ptr<NewCallModel>(new NewCallModel(*this, info));
        info.contactModel = std::unique_ptr<ContactModel>(new ContactModel(*this, database, callbackHandler, info));
        info.conversationModel = std::unique_ptr<ConversationModel>(new ConversationModel(*this, database, info));
    }
}

NewAccountModel::~NewAccountModel()
{
}

const std::vector<std::string>
NewAccountModel::getAccountList() const
{
    std::vector<std::string> accountsId;

    for(auto const& accountInfo: pimpl_->accounts)
        accountsId.push_back(accountInfo.first);

    return std::move(accountsId);
}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId) const
{
    return pimpl_->accounts[accountId];
}

NewAccountModelPimpl::NewAccountModelPimpl(const Database& database, const CallbacksHandler& callbackHandler)
: database(database)
, callbackHandler(callbackHandler)
{

}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

} // namespace api
} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
