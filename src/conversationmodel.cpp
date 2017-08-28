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

namespace lrc
{

ConversationModel::ConversationModel(QObject* parent)
:QObject(parent)
{

}

ConversationModel::~ConversationModel()
{

}

std::shared_ptr<ContactModel>
ConversationModel::getContactModel()
{
    return contactModel_;
}


const lrc::ConversationsList&
ConversationModel::getConversations() const
{
    return lrc::ConversationsList();
}

std::shared_ptr<lrc::conversation::Info>
ConversationModel::getConversation(const unsigned int row) const
{
    if (row >= filteredConversations_.size())
        throw std::out_of_range("Can't get conversation at row " + std::to_string(row));
    return filteredConversations_.at(row);
}

const conversation::Info&
ConversationModel::addConversation(const std::string& uri)
{
    return conversation::Info();
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
ConversationModel::setFilter(const std::string&)
{

}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{

}

void
ConversationModel::cleanHistory(const std::string& uid)
{

}

}
