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

// Std
#include <string>
#include <memory>

// Lrc
#include <api/contact.h>

namespace lrc
{

namespace api
{

class NewCallModel;
class ContactModel;
class ConversationModel;
class NewAccountModel;

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
    contact::Info profile;  // TODO: not the best, we should separate profile and contact::Info...
    Type type = account::Type::INVALID;
    std::unique_ptr<lrc::api::NewCallModel> callModel;
    std::unique_ptr<lrc::api::ContactModel> contactModel;
    std::unique_ptr<lrc::api::ConversationModel> conversationModel;
    std::unique_ptr<const lrc::api::NewAccountModel> accountModel;
};

} // namespace account
} // namespace api
} // namespace lrc
