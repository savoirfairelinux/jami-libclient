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

NewAccountModel::NewAccountModel(std::shared_ptr<DatabaseManager> dbManager)
:QObject(nullptr)
, dbManager_(dbManager)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto id : accountIds) {
        // first we build all objects contained in the info structure
        auto callModel = std::make_shared<NewCallModel>();
        auto contactModel = std::make_shared<ContactModel>();
        auto conversationModel = std::make_shared<ConversationModel>();
        
        auto info = std::make_shared<NewAccount::Info>(id.toStdString(), callModel, contactModel, conversationModel);
        accounts_[id.toStdString()] = info;
    }

}

NewAccountModel::~NewAccountModel()
{

}

pAccountInfo
NewAccountModel::getAccountInfo(const std::string& id)
{
    return accounts_[id];
}
