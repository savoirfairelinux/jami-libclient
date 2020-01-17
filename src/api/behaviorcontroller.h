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

// Std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"

namespace lrc
{

class BehaviorControllerPimpl;

namespace api
{
class Lrc;

namespace conversation
{
    struct Info;
}

namespace interaction
{
    struct Info;
}

/**
  *  @brief Class that helps to control behaviors from the client side.
  *  @note This class must only refer to the common behaviors.
  */
class LIB_EXPORT BehaviorController : public QObject {
    Q_OBJECT

public:
    BehaviorController();
    ~BehaviorController();

Q_SIGNALS:
    /**
     * Emitted when the client should open the chat view.
     */
    void showChatView(const std::string& accountId, const api::conversation::Info& conversationInfo) const;
    /**
     * Emitted when the client should ask the user whether it wants to leave a message after a failed call.
     */
    void showLeaveMessageView(const std::string& accountId, const api::conversation::Info& conversationInfo) const;
    /**
     * Emitted when the client should open the call view.
     */
    void showCallView(const std::string& accountId, const api::conversation::Info& conversationInfo) const;
    /**
     * Emitted when the client should open the incoming call view.
     */
    void showIncomingCallView(const std::string& accountId, const api::conversation::Info& conversationInfo) const;
    /**
     * Emitted when the client receives a new trust request
     */
    void newTrustRequest(const std::string& accountId, const std::string& contactUri) const;
    /**
     * Emitted when a trust request has been accepted, refused or blocked
     */
    void trustRequestTreated(const std::string& accountId, const std::string& contactUri) const;
    /**
     * Emitted when the client receives an unread message to display (text or file for now)
     */
    void newUnreadInteraction(const std::string& accountId, const std::string& conversation,
        uint64_t interactionId, const interaction::Info& interaction) const;
    /**
     * Emitted when the unread interaction is now read
     */
     void newReadInteraction(const std::string& accountId, const std::string& conversation, uint64_t interactionId) const;
     /**
     * Emitted debugMessageReceived
     */
     void debugMessageReceived(const std::string& message);
     /**
     * Emitted audioMeter
     */
     void audioMeter(const std::string& id, float level);

};

} // namespace api
} // namespace lrc
