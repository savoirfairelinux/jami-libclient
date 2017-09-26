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
#include <ctime>
#include <string>

namespace lrc
{

namespace api
{

namespace message
{

enum class Type {
    INVALID,
    TEXT,
    CALL,
    CONTACT
};

static const std::string
TypeToString(Type type)
{
    switch(type) {
    case Type::TEXT:
        return "TEXT";
    case Type::CALL:
        return "CALL";
    case Type::CONTACT:
        return "CONTACT";
    case Type::INVALID:
    default:
        return "INVALID";
    }
}

static Type
StringToType(const std::string& type)
{
    if (type == "TEXT") {
        return message::Type::TEXT;
    } else if (type == "CALL") {
        return message::Type::CALL;
    } else if (type == "CONTACT") {
        return message::Type::CONTACT;
    } else
        return message::Type::INVALID;
}


enum class Status {
    INVALID,
    SENDING,
    FAILED,
    SUCCEED,
    READ,
    UNREAD
};

static const std::string
StatusToString(Status status)
{
    switch(status) {
    case Status::SENDING:
        return "SENDING";
    case Status::FAILED:
        return "FAILED";
    case Status::SUCCEED:
        return "SUCCEED";
    case Status::READ:
        return "READ";
    case Status::UNREAD:
        return "UNREAD";
    case Status::INVALID:
    default:
        return "INVALID";
    }
}

static Status
StringToStatus(const std::string& status)
{
    if (status == "SENDING")
        return message::Status::SENDING;
    else if (status == "FAILED")
        return message::Status::FAILED;
    else if (status == "SUCCEED")
        return message::Status::SUCCEED;
    else if (status == "READ")
        return message::Status::READ;
    else if (status == "UNREAD")
        return message::Status::UNREAD;
    else
        return message::Status::INVALID;

}

struct Info
{
    const std::string authorUri;
    const std::string body;
    const std::time_t timestamp = 0;
    Type type = Type::INVALID;
    Status status = Status::INVALID;
};

static bool isOutgoing(const Info& message) {
    return message.status != lrc::api::message::Status::READ
    && message.status != lrc::api::message::Status::UNREAD;
}

} // namespace message
} // namespace api
} // namespace lrc
