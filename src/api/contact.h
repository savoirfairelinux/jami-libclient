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

namespace lrc
{

namespace api
{

namespace contact
{

enum class Type {
    INVALID,
    RING,
    SIP,
    PENDING,
};

static const std::string
TypeToString(Type type)
{
    switch(type) {
    case Type::INVALID:
        return "INVALID";
    case Type::RING:
        return "RING";
    case Type::SIP:
        return "SIP";
    case Type::PENDING:
        return "PENDING";
    }

    //throw something
}

static Type
StringToType(const std::string& type)
{
    if (type == "PENDING") {
        return contact::Type::PENDING;
    } else if (type == "SIP") {
        return contact::Type::SIP;
    } else if (type == "RING") {
        return contact::Type::RING;
    } else if (type == "INVALID")
        return contact::Type::INVALID;
    //throw something
}

struct Info
{
    std::string uri;
    std::string avatar;
    std::string registeredName;
    std::string alias;
    bool isTrusted = false;
    bool isPresent = false;
    Type type = Type::INVALID;
};

} // namespace contact
} // namespace api
} // namespace lrc
