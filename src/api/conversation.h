/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                                  *
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

#include "interaction.h"
#include "typedefs.h"

#include <vector>
#include <map>

namespace lrc {

namespace api {

namespace conversation {

struct Info
{
    Info() = default;
    Info(const Info& other) = delete;
    Info(Info&& other) = default;
    Info& operator=(const Info& other) = delete;
    Info& operator=(Info&& other) = default;

    QString uid = "";
    QString accountId;
    VectorString participants;
    QString callId;
    QString confId;
    std::map<uint64_t, interaction::Info> interactions;
    uint64_t lastMessageUid = 0;
    std::map<QString, uint64_t> lastDisplayedMessageUid;
    unsigned int unreadMessages = 0;

    QString getConfIdOrCallId() {
        return confId.isEmpty() ? callId : confId;
    }
};

} // namespace conversation
} // namespace api
} // namespace lrc
