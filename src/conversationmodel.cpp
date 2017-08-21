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
#include "conversationmodel.h"

// std
#include <regex>

// LRC
#include "availableaccountmodel.h"
#include "contactmethod.h"
#include "dbus/configurationmanager.h"
#include "phonedirectorymodel.h"

namespace lrc
{

ConversationModel::ConversationModel(NewCallModel& callModel,
                                     ContactModel& contactModel,
                                     const Database& database)
: callModel_(callModel)
, contactModel_(contactModel)
, database_(database)
, QObject()
{

}

ConversationModel::~ConversationModel()
{

}

ContactModel&
ConversationModel::getContactModel() const
{
    return contactModel_;
}


const ConversationsList&
ConversationModel::getFilteredConversations() const
{
    return conversations_;
}

conversation::Info
ConversationModel::getConversation(const unsigned int row) const
{

}

void
ConversationModel::addConversation(const std::string& uri) const
{

}

void
ConversationModel::selectConversation(const std::string& uid)
{

}

void
ConversationModel::removeConversation(const std::string& uid)
{

}

void
ConversationModel::placeCall(const std::string& uid) const
{

}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{

}

void
ConversationModel::setFilter(const std::string& filter)
{

}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{

}

void
ConversationModel::clearHistory(const std::string& uid)
{

}

void
ConversationModel::initConversations()
{

}

void
ConversationModel::sortConversations()
{

}

void
ConversationModel::slotContactsChanged()
{

}

void
ConversationModel::slotMessageAdded(int uid, const std::string& account, message::Info msg)
{

}

int
ConversationModel::find(const std::string& uid) const
{

}

void
ConversationModel::search()
{

}

void
ConversationModel::registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name)
{

}

} // namespace lrc
