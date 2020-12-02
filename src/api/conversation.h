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
#include "messageslist.h"
#include "typedefs.h"

#include <vector>
#include <map>

namespace lrc {

namespace api {

namespace conversation {

enum class Mode { ONE_TO_ONE, ADMIN_INVITES_ONLY, INVITES_ONLY, PUBLIC, NON_SWARM };

static inline Mode
to_mode(const int intMode)
{
    switch (intMode) {
        case 0:
            return Mode::ONE_TO_ONE;
        case 1:
            return Mode::ADMIN_INVITES_ONLY;
        case 2:
            return Mode::INVITES_ONLY;
        case 3:
            return Mode::PUBLIC;
        case 4:
            return Mode::NON_SWARM;
        default:
            return Mode::ONE_TO_ONE;
    }
}

struct Info
{
    Info() = default;
    Info(const Info& other) = delete;
    Info(Info&& other) = default;
    Info& operator=(const Info& other) = delete;
    Info& operator=(Info&& other) = default;

    bool allMessagesLoaded = false;
    QString uid = "";
    QString accountId;
    VectorString participants;
    QString callId;
    QString confId;
    MessagesList interactions;
    QString lastMessageUid = 0;
    QHash<QString, QString> parentsId; // pair messageid/parentid for messages without parent loaded
    std::map<QString, QString> lastDisplayedMessageUid;
    unsigned int unreadMessages = 0;

    QString getCallId() { return confId.isEmpty() ? callId : confId; }

    Mode mode = Mode::NON_SWARM;
    bool isRequest = false;
};

} // namespace conversation
} // namespace api
} // namespace lrc
