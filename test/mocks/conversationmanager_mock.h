/*
 *  Copyright (C) 2020 Savoir-faire Linux Inc.
 *
 *  Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */
#pragma once

#include <QtCore/QObject>

#include <conversation_interface.h>
#include "qtwrapper/conversions_wrap.hpp"

/*
 * Proxy class for interface org.ring.Ring.ConversationManager
 */
class ConversationManagerInterface : public QObject
{
    Q_OBJECT
public:
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> conversationsHandlers {};

    ConversationManagerInterface() {}

    ~ConversationManagerInterface() {}
    bool isValid() { return true; }

public Q_SLOTS: // METHODS
    QString startConversation(const QString& accountId)
    {
        Q_UNUSED(accountId);
        return "";
    }
    void acceptConversationRequest(const QString& accountId, const QString& conversationId)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
    }
    void declineConversationRequest(const QString& accountId, const QString& conversationId)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
    }
    bool removeConversation(const QString& accountId, const QString& conversationId)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
        return true;
    }
    QStringList getConversations(const QString& accountId)
    {
        Q_UNUSED(accountId);
        return QStringList();
    }
    VectorMapStringString getConversationRequests(const QString& accountId)
    {
        Q_UNUSED(accountId);
        return VectorMapStringString();
    }
    bool addConversationMember(const QString& accountId,
                               const QString& conversationId,
                               const QString& memberURI)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
        Q_UNUSED(memberURI);
        return true;
    }
    bool removeConversationMember(const QString& accountId,
                                  const QString& conversationId,
                                  const QString& memberURI)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
        Q_UNUSED(memberURI);
        return true;
    }
    VectorMapStringString getConversationMembers(const QString& accountId,
                                                 const QString& conversationId)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
        return VectorMapStringString();
    }
    void sendMessage(const QString& accountId,
                     const QString& conversationId,
                     const QString& message,
                     const QString& parrent)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
        Q_UNUSED(message);
        Q_UNUSED(parrent);
    }
    uint32_t loadConversationMessages(const QString& accountId,
                                      const QString& conversationId,
                                      const QString& fromId,
                                      const int size)
    {
        Q_UNUSED(accountId);
        Q_UNUSED(conversationId);
        Q_UNUSED(fromId);
        Q_UNUSED(size);
        return 0;
    }

Q_SIGNALS: // SIGNALS
    void conversationLoaded(uint32_t loadingRequestId,
                            const QString& accountId,
                            const QString& conversationId,
                            VectorMapStringString messages);
    void messageReceived(const QString& accountId,
                         const QString& conversationId,
                         MapStringString message);
    void conversationRequestReceived(const QString& accountId,
                                     const QString& conversationId,
                                     MapStringString metadatas);
    void conversationReady(const QString& accountId, const QString& conversationId);
};

namespace org {
namespace ring {
namespace Ring {
typedef ::ConversationManagerInterface ConversationManager;
}
} // namespace ring
} // namespace org
