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

#include "typedefs.h"

#include <QObject>

#include <memory>

namespace lrc {

class BehaviorControllerPimpl;

namespace api {
class Lrc;

namespace conversation {
struct Info;
}

namespace interaction {
struct Info;
}

/**
 *  @brief Class that helps to control behaviors from the client side.
 *  @note This class must only refer to the common behaviors.
 */
class LIB_EXPORT BehaviorController : public QObject
{
    Q_OBJECT

public:
    BehaviorController();
    ~BehaviorController();

Q_SIGNALS:
    /**
     * Emitted when the client should open the chat view.
     */
    void showChatView(const QString& accountId, const QString& convUid) const;
    /**
     * Emitted when the client should ask the user whether it wants to leave a message after a
     * failed call.
     */
    void showLeaveMessageView(const QString& accountId, const QString& convUid) const;
    /**
     * Emitted when the client should open the call view.
     */
    void showCallView(const QString& accountId, const QString& convUid) const;
    /**
     * Emitted when the client should open the incoming call view.
     */
    void showIncomingCallView(const QString& accountId, const QString& convUid) const;
    /**
     * Emitted when the client receives a new trust request
     */
    void newTrustRequest(const QString& accountId, const QString& conversationId, const QString& contactUri) const;
    /**
     * Emitted when a trust request has been accepted, refused or blocked
     */
    void trustRequestTreated(const QString& accountId, const QString& contactUri) const;
    /**
     * Emitted when the client receives an unread message to display (text or file for now)
     */
    void newUnreadInteraction(const QString& accountId,
                              const QString& conversation,
                              const QString& interactionId,
                              const interaction::Info& interaction) const;
    /**
     * Emitted when the unread interaction is now read
     */
    void newReadInteraction(const QString& accountId,
                            const QString& conversation,
                            const QString& interactionId) const;
    /**
     * Emitted debugMessageReceived
     */
    void debugMessageReceived(const QString& message);
    /**
     * Emitted audioMeter
     */
    void audioMeter(const QString& id, float level);

    /**
     * Emitted callStatusChanged
     */
    void callStatusChanged(const QString& accountId, const QString& callId) const;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::BehaviorController*)
