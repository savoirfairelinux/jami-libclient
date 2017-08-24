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
#pragma once
#include "message.h"
#include "callinfo.h"
#include "contactinfo.h"
#include "account.h" // old

// std
#include <memory>
#include <deque>

namespace Conversation
{

struct Info
{
    const std::string uid_= "";
    std::vector <std::shared_ptr<Contact::Info>> participants_;
    std::shared_ptr<NewCall::Info> call_;
    Messages messages_;
    int lastMessageUid_ = -1;
    bool isUsed_ = false;
    Account* account_; // old
    unsigned int index_ = 0;
    unsigned int unreadMessages = 0;

    // create a new converation when a search was made.
    Info(Account* account, std::shared_ptr<Contact::Info> participant)
    : account_(account), participants_({participant}){}

    // create a new conversation when we load data from database.
    Info(Account* account, const std::string& uid, std::vector <std::shared_ptr<Contact::Info>>& participants, Messages messages)
    : account_(account), uid_(uid), participants_(participants), messages_(messages) {
        if(!messages_.empty()) {
            lastMessageUid_ = (--messages_.end())->first;
            isUsed_ = true;
        }
    }
    Info(Account* account, const std::string& uid, std::shared_ptr<Contact::Info> participant, Messages messages)
    : account_(account), uid_(uid), participants_({participant}), messages_(messages) {
        if(!messages_.empty()) {
            lastMessageUid_ = (--messages_.end())->first;
            isUsed_ = true;
        }
    }

    // create a new conversation when someone is calling for the first time.
    Info(Account* account, const std::string& uid, std::shared_ptr<Contact::Info> participant, std::shared_ptr<NewCall::Info> call)
    : account_(account), uid_(uid), participants_({participant}), call_(call) {}

};

}

typedef std::pair<std::string, std::shared_ptr<Conversation::Info>> ConversationEntry;
typedef std::deque<ConversationEntry> Conversations;
