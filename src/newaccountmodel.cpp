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

#include "dbus/configurationmanager.h" // old

namespace lrc
{

NewAccountModel::NewAccountModel(std::unique_ptr<DatabaseManager>& dbManager)
:QObject(nullptr)
, dbManager_(dbManager)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds) {
        QMap<QString, QString> details = ConfigurationManager::instance().getAccountDetails(id);

        account::Info info;
        info.id = id.toStdString();
        info.type = details["Account.type"] == "RING" ? account::Type::RING : account::Type::SIP;
        info.callModel = std::make_shared<NewCallModel>();
        info.contactModel = std::make_shared<ContactModel>(*dbManager_.get(), id.toStdString());
        info.conversationModel = std::make_shared<ConversationModel>(info.callModel, info.contactModel, *dbManager_.get());

        accounts_[id.toStdString()] = std::move(info);
    }

}

NewAccountModel::~NewAccountModel()
{

}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& id)
{
    return accounts_[id];
}

} // namespace lrc
