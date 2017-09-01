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
#include "api/conversationmodel.h"

// std
#include <regex>

// Models and database
#include "database.h"
#include "api/newcallmodel.h"
#include "api/newaccountmodel.h"

namespace lrc
{

namespace api
{

class ConversationModelPimpl
{
public:
    ConversationModelPimpl(const NewAccountModel& p, const Database& d, const account::Info& o);
    ~ConversationModelPimpl();

    // shortcuts in owner
    NewCallModel& callModel;

    /**
     * Search a conversation in conversations_
     * @param uid the contact to search
     * @return the index in conversations_
     */
    int find(const std::string& uid) const;
    /**
     * Initialize conversations_ and filteredConversations_
     */
    void initConversations();
    /**
     * Sort conversation by last action
     */
    void sortConversations();
    void search();

    const NewAccountModel& parent;
    const Database& database;

    ConversationQueue conversations;
    mutable ConversationQueue filteredConversations;
    std::string filter;

};

ConversationModel::ConversationModel(const NewAccountModel& parent,
                                     const Database& database,
                                     const account::Info& info)
: pimpl_(std::make_unique<ConversationModelPimpl>(parent, database, owner))
, owner(info)
{

}

ConversationModel::~ConversationModel()
{

}

const ConversationQueue&
ConversationModel::getFilteredConversations() const
{
    return pimpl_->conversations;
}

conversation::Info
ConversationModel::getConversation(const unsigned int row) const
{
    return {};
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
ConversationModel::slotContactsChanged()
{

}

void
ConversationModel::slotMessageAdded(int uid, const std::string& account, const message::Info& msg)
{

}

void
ConversationModel::registeredNameFound(const Account* account, NameDirectory::LookupStatus status,
                                       const QString& address, const QString& name)
{

}

ConversationModelPimpl::ConversationModelPimpl(const NewAccountModel& p, const Database& d, const account::Info& o)
: parent(p)
, database(d)
, callModel(*o.callModel)
{

}

ConversationModelPimpl::~ConversationModelPimpl()
{

}

int
ConversationModelPimpl::find(const std::string& uid) const
{
    return -1;
}

void
ConversationModelPimpl::search()
{

}

void
ConversationModelPimpl::initConversations()
{

}

void
ConversationModelPimpl::sortConversations()
{

}

} // namespace api
} // namespace lrc
