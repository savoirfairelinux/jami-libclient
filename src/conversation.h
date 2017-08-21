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

namespace Conversation
{

struct Info
{
    const std::string uid_= "";
    std::vector <Contact::Info> participants_;
    NewCall::Info call_ = NewCall::Info(0, 0, NewCall::Status::INVALID_STATUS);
    Messages messages_;
    bool isUsed_ = false;
    Account* account_; // old
    unsigned int index_;

    // create a new converation when a search was made.
    Info(Account* account, Contact::Info participant)
    : account_(account), participants_({participant}){}
    
    // create a new conversation when we load data from database.
    Info(Account* account, const std::string& uid, std::vector <Contact::Info> participants, Messages messages)
    : account_(account), uid_(uid), participants_(participants), messages_(messages) {}
    // utiliser le constructeur pour ouvrir la discussion sur les chatsgroupes/conferences.
    Info(Account* account, const std::string& uid, Contact::Info participant, Messages messages)
    : account_(account), uid_(uid), participants_({participant}), messages_(messages) {}
    
    // create a new conversation when someone is calling for the first time.
    Info(Account* account, const std::string& uid, Contact::Info participant, NewCall::Info call)
    : account_(account), uid_(uid), participants_({participant}), call_(call) {}
    // discuter avec guillaume le cas du client qui crash et pas le daemon durant un appel.
    // discuter avec " " " " " pour la conference.

};

}

typedef std::deque<std::pair<std::string, std::shared_ptr<Conversation::Info>>> Conversations;
