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

namespace lrc
{

namespace conversation
{

struct Info
{
    std::string uid = "";
    std::vector <std::shared_ptr<contact::Info>> participants;
    std::shared_ptr<call::Info> call;
    lrc::MessagesMap messages;
    int lastMessageUid = -1;
    bool isUsed = false;
    Account* account; // old
    unsigned int unreadMessages = 0;
};

} // namespace conversation

using ConversationsList = std::deque<std::shared_ptr<conversation::Info>>;

} //namespace lrc
