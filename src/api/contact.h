/****************************************************************************
 *    Copyright (C) 2017-2020 Savoir-faire Linux Inc.                                  *
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

#include "profile.h"

#include <QString>

namespace lrc
{

namespace api
{

namespace contact
{

/**
 * @var profileInfo
 * @var registeredName
 * @var isTrusted
 * @var isPresent
 * @var isBanned
 */
struct Info
{
    profile::Info profileInfo;
    QString registeredName;
    bool isTrusted = false;
    bool isPresent = false;
    bool isBanned = false;
};

} // namespace contact
} // namespace api
} // namespace lrc
