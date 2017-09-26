/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// Data
#include "profile.h"

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

enum class Status {
    INVALID,
    INITIALIZING,
    UNREGISTERED,
    TRYING,
    REGISTERED
};

static account::Status
StringToStatus(const std::string& type)
{
    if (type == "INITIALIZING")
        return account::Status::INITIALIZING;
    else if (type == "UNREGISTERED")
        return account::Status::UNREGISTERED;
    else if (type == "TRYING")
        return account::Status::TRYING;
    else if (type == "REGISTERED")
        return account::Status::REGISTERED;
    else
        return account::Status::INVALID;
}

struct Info
{
    std::string id;
    std::string registeredName;
    bool enabled;
    Status status = account::Status::INVALID;
    profile::Info profileInfo;
    std::unique_ptr<lrc::api::NewCallModel> callModel;
    std::unique_ptr<lrc::api::ContactModel> contactModel;
    std::unique_ptr<lrc::api::ConversationModel> conversationModel;
    NewAccountModel* accountModel {nullptr};
};

} // namespace account
} // namespace api
} // namespace lrc
