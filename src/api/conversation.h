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
#include "messagelistmodel.h"
#include "typedefs.h"

#include <map>
#include <memory>
#include <vector>

namespace lrc {

namespace api {

namespace conversation {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

enum class Mode { ONE_TO_ONE, ADMIN_INVITES_ONLY, INVITES_ONLY, PUBLIC, NON_SWARM };
Q_ENUM_NS(Mode)

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
    Info()
        : interactions(std::make_unique<MessageListModel>(nullptr))
    {}
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
    std::unique_ptr<MessageListModel> interactions;
    // MessageListModel interactions;
    QString lastMessageUid = 0;
    QHash<QString, QString> parentsId; // pair messageid/parentid for messages without parent loaded
    unsigned int unreadMessages = 0;

    QSet<QString> typers;

    MapStringString infos {};

    QString getCallId() const { return confId.isEmpty() ? callId : confId; }

    inline bool isLegacy() const { return mode == Mode::NON_SWARM; }
    inline bool isSwarm() const { return !isLegacy(); }
    // for each contact we must have one non-swarm conversation or one active one-to-one
    // conversation. Where active means peer did not leave the conversation.
    inline bool isCoreDialog() const { return isLegacy() || mode == Mode::ONE_TO_ONE; };

    Mode mode = Mode::NON_SWARM;
    bool needsSyncing = false;
    bool isRequest = false;
    bool readOnly = false;
};

} // namespace conversation
} // namespace api
} // namespace lrc
