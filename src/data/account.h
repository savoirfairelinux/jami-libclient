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

namespace lrc
{

class NewCallModel;
class ContactModel;
class ConversationModel;
class NewAccountModel;

using upNewCallModel = std::shared_ptr<NewCallModel>;
using upContactModel = std::shared_ptr<ContactModel>;
using upConversationModel = std::shared_ptr<ConversationModel>;
using upNewAccountModel = std::shared_ptr<NewAccountModel>;

namespace account
{

enum class Type {
    INVALID,
    RING,
    SIP
};

struct Info
{
    std::string id;
    Type type = account::Type::INVALID;
    upNewCallModel callModel;
    upContactModel contactModel;
    upConversationModel conversationModel;
    upNewAccountModel accountModel;
};

} // namespace account

using AccountsInfoMap = std::map<std::string, std::shared_ptr<account::Info>>;

} // namespace lrc
