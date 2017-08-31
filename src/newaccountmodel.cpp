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
    NewAccountModelPimpl(const Database& database);
    ~NewAccountModelPimpl();

    const Database& database;
    AccountInfoMap accounts;
};

NewAccountModel::NewAccountModel(const Database& database)
: pimpl_(std::make_unique<NewAccountModelPimpl>(database))
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

        pimpl_->accounts[id.toStdString()] = std::move(info);
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
NewAccountModel::getAccountInfo(const std::string& accountId)
{
    return pimpl_->accounts[accountId];
}

NewAccountModelPimpl::NewAccountModelPimpl(const Database& database)
: database(database)
{

}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

} // namespace api
} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
