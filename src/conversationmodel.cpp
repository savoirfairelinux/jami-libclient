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

// LRC
#include "api/newcallmodel.h"
#include "api/newaccountmodel.h"
#include "api/account.h"

#include "database.h"
#include "namedirectory.h"

namespace lrc
{

using namespace api;

class ConversationModelPimpl : public QObject
{
    Q_OBJECT
public:
    ConversationModelPimpl(const ConversationModel& linked, const Database& db);
    ~ConversationModelPimpl();

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

    const ConversationModel& linked;
    const Database& db;

    ConversationModel::ConversationQueue conversations;
    mutable ConversationModel::ConversationQueue filteredConversations;
    std::string filter;

public Q_SLOTS:
    void slotContactsChanged();
    void slotMessageAdded(int uid, const std::string& accountId, const message::Info& msg);
    void slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
                             const QString& address, const QString& name);

};

ConversationModel::ConversationModel(const account::Info& owner, const Database& database)
: QObject()
, pimpl_(std::make_unique<ConversationModelPimpl>(*this, database))
, owner(owner)
{

}

ConversationModel::~ConversationModel()
{

}

const ConversationModel::ConversationQueue&
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

ConversationModelPimpl::ConversationModelPimpl(const ConversationModel& linked, const Database& db)
: linked(linked)
, db(db)
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

void
ConversationModelPimpl::slotContactsChanged()
{

}

void
ConversationModelPimpl::slotMessageAdded(int uid, const std::string& account, const message::Info& msg)
{

}

void
ConversationModelPimpl::slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
                                           const QString& address, const QString& name)
{

}

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"
