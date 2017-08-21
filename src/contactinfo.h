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

namespace Contact
{

enum class Type {
    RING,
    SIP,
    INVALID_TYPE
};

struct Info
{
    const std::string uri_;
    std::string avatar_;
    std::string registeredName_;
    std::string alias_;
    bool isTrusted_;
    Type type_;

    Info(const std::string& uri = "", const std::string& avatar = "", const std::string& registeredName = "",
         const std::string& alias = "", bool isTrusted = false, Type type = Type::INVALID_TYPE)
        : uri_(uri), avatar_(avatar), registeredName_(registeredName), alias_(alias), isTrusted_(isTrusted), type_(type)
        {}
};

}

typedef std::map<std::string, std::shared_ptr<Contact::Info>> ContactsInfo;
