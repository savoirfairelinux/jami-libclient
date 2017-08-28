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
    case Type::INVALID:
        return "INVALID";
    case Type::TEXT:
        return "TEXT";
    case Type::CALL:
        return "CALL";
    case Type::CONTACT:
        return "CONTACT";
    }

    //throw something
}

enum class Status {
    INVALID,
    SENDING,
    FAILED,
    SUCCEED,
    READ
};

static const std::string
StatusToString(Status status)
{
    switch(status) {
    case Status::INVALID:
        return "INVALID";
    case Status::SENDING:
        return "SENDING";
    case Status::FAILED:
        return "FAILED";
    case Status::SUCCEED:
        return "SUCCEED";
    case Status::READ:
        return "READ";
    }

    //throw something
}

struct Info
{
    std::string contact; // [jn] const ?, to rename with something more like contactUri
    std::string body; // [jn] const ?
    std::time_t timestamp = 0; // [jn] const ?
    Type type = Type::INVALID;
    Status status = Status::INVALID;
};

} // namespace message
} // namespace api
} // namespace lrc
