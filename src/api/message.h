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

enum class Status {
    INVALID,
    SENDING,
    FAILED,
    SUCCEED,
    READ
};

struct Info
{
    std::string contact; // [jn] const ?
    std::string body; // [jn] const ?
    std::time_t timestamp = 0; // [jn] const ?
    Type type = Type::INVALID;
    Status status = Status::INVALID;
};

static bool isOutgoing(const Info& message) {
    return message.status != lrc::api::message::Status::READ;
}

} // namespace message
} // namespace api
} // namespace lrc
