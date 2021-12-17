/****************************************************************************
 *    Copyright (C) 2021 Savoir-faire Linux Inc.                            *
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

#include "typedefs.h"

#include <map>
#include <memory>
#include <vector>

namespace lrc {

namespace api {

namespace member {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

enum class Role { ADMIN, MEMBER, INVITED, BANNED, LEFT };
Q_ENUM_NS(Role)

static inline Role
to_role(const QString& roleStr)
{
    if (roleStr == "admin")
        return Role::ADMIN;
    if (roleStr == "member")
        return Role::MEMBER;
    if (roleStr == "invited")
        return Role::INVITED;
    if (roleStr == "banned")
        return Role::BANNED;
    if (roleStr == "left")
        return Role::LEFT;
    return Role::MEMBER;
}

struct Member
{
    QString uri = "";
    Role role = Role::MEMBER;
};

} // namespace member
} // namespace api
} // namespace lrc
