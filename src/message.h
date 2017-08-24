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

// std
#include <ctime>
#include <string>
#include <map>

namespace Message
{

enum class Type {
    TEXT,
    CALL,
    CONTACT,
    INVALID_TYPE
};

enum class Status {
    SENDING,
    FAILED,
    SUCCEED,
    INVALID_STATUS
};

struct Info
{
    const std::string uid_;
    const std::string body_;
    const std::time_t timestamp_;
    const bool isOutgoing_;
    const Type type_;
    Status status_;

    Info(const std::string& uid, const std::string& body, bool isOutgoing, Type type
    , const std::time_t timestamp = std::time(nullptr), Status status = Status::INVALID_STATUS)
    : uid_(uid), body_(body), timestamp_(timestamp), isOutgoing_(isOutgoing), type_(type), status_(status)
    {};
};

}

typedef std::map<int, Message::Info> Messages;
