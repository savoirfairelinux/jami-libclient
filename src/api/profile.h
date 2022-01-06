/****************************************************************************
 *    Copyright (C) 2017-2022 Savoir-faire Linux Inc.                       *
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

#include <QObject>
#include <QString>

namespace lrc {

namespace api {

namespace profile {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

enum class Type { INVALID, JAMI, SIP, PENDING, TEMPORARY, COUNT__ };
Q_ENUM_NS(Type)

static inline const QString
to_string(const Type& type)
{
    switch (type) {
    case Type::JAMI:
        return "JAMI";
    case Type::SIP:
        return "SIP";
    case Type::PENDING:
        return "PENDING";
    case Type::TEMPORARY:
        return "TEMPORARY";
    case Type::INVALID:
    case Type::COUNT__:
    default:
        return "INVALID";
    }
}

static inline Type
to_type(const QString& type)
{
    if (type == "PENDING")
        return Type::PENDING;
    else if (type == "SIP")
        return Type::SIP;
    else if (type == "JAMI")
        return Type::JAMI;
    else if (type == "TEMPORARY")
        return Type::TEMPORARY;
    else
        return Type::INVALID;
}

/**
 * @var uri
 * @var avatar
 * @var alias
 * @var type
 */
struct Info
{
    QString uri = "";
    QString avatar = "";
    QString alias = "";
    Type type = Type::INVALID;
};

} // namespace profile
} // namespace api
} // namespace lrc
