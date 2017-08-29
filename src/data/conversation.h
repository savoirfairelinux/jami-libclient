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
#include "data/message.h"
#include "data/call.h"
#include "data/contact.h"

// std
#include <memory>
#include <deque>
#include <vector>

namespace lrc
{

namespace conversation
{

struct Info
{
    std::string uid;
    std::string accountId;
    std::vector<contact::Info> participants;
    //call::Info call;
    MessagesMap messages;
    unsigned int lastMessageUid = 0;
    bool isUsed = false;
    unsigned int unreadMessages = 0;
};

} // namespace conversation

using ConversationsList = std::deque<conversation::Info>;

} //namespace lrc
