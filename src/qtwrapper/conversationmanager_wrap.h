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

#include <conversation_interface.h>

/*
 * Proxy class for interface org.ring.Ring.ConversationManager
 */
class ConversationManagerInterface : public QObject
{
    Q_OBJECT

public:
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> conversationsHandlers;

    ConversationManagerInterface()
    {
        setObjectName("ConversationManagerInterface");
        using DRing::exportable_callback;
        using DRing::ConversationSignal;

        conversationsHandlers = {
            exportable_callback<ConversationSignal::ConversationLoaded>(
                [this](uint32_t loadingRequestId,
                       const std::string& accountId,
                       const std::string& conversationId,
                       const std::vector<std::map<std::string, std::string>>& messages) {
                    Q_EMIT this->messagesLoaded(loadingRequestId,
                                                QString(accountId.c_str()),
                                                QString(conversationId.c_str()),
                                                convertVecMap(messages));
                }),
            exportable_callback<ConversationSignal::MessageReceived>(
                [this](const std::string& accountId,
                       const std::string& conversationId,
                       const std::map<std::string, std::string>& message) {
                    Q_EMIT this->messageReceived(QString(accountId.c_str()),
                                                 QString(conversationId.c_str()),
                                                 convertMap(message));
                }),
            exportable_callback<ConversationSignal::ConversationRequestReceived>(
                [this](const std::string& accountId,
                       const std::string& conversationId,
                       const std::map<std::string, std::string>& metadata) {
                    Q_EMIT this->conversationRequestReceived(QString(accountId.c_str()),
                                                             QString(conversationId.c_str()),
                                                             convertMap(metadata));
                }),
            exportable_callback<ConversationSignal::ConversationReady>(
                [this](const std::string& accountId, const std::string& conversationId) {
                    Q_EMIT this->conversationReady(QString(accountId.c_str()),
                                                   QString(conversationId.c_str()));
                }),
        };
    }

    ~ConversationManagerInterface() {}

public Q_SLOTS: // METHODS
    void startConversation(const std::string& accountId) { DRing::startConversation(accountId); }
    void acceptConversationRequest(const std::string& accountId, const std::string& conversationId)
    {
        DRing::acceptConversationRequest(accountId, conversationId);
    }
    void declineConversationRequest(const std::string& accountId, const std::string& conversationId)
    {
        DRing::declineConversationRequest(accountId, conversationId);
    }
    bool removeConversation(const std::string& accountId, const std::string& conversationId)
    {
        return DRing::removeConversation(accountId, conversationId);
    }
    std::vector<std::string> getConversations(const std::string& accountId)
    {
        return DRing::getConversations(accountId);
    }
    std::vector<std::map<std::string, std::string>> getConversationRequests(
        const std::string& accountId)
    {
        return DRing::getConversationRequests(accountId);
    }
    bool addConversationMember(const std::string& accountId,
                               const std::string& conversationId,
                               const std::string& memberURI)
    {
        return DRing::addConversationMember(accountId, conversationId, memberURI);
    }
    bool removeConversationMember(const std::string& accountId,
                                  const std::string& conversationId,
                                  const std::string& memberURI)
    {
        return DRing::removeConversationMember(accountId, conversationId, memberURI);
    }
    std::vector<std::map<std::string, std::string>> getConversationMembers(
        const std::string& accountId, const std::string& conversationId)
    {
        return DRing::getConversationMembers(accountId, conversationId);
    }
    void sendMessage(const std::string& accountId,
                     const std::string& conversationId,
                     const std::string& message,
                     const std::string& parrent)
    {
        DRing::sendMessage(accountId, conversationId, message, parrent);
    }
    uint32_t loadConversationMessages(const std::string& accountId,
                                      const std::string& conversationId,
                                      const std::string& fromId,
                                      const int size)
    {
        return DRing::loadConversationMessages(accountId, conversationId, fromId, size);
    }

Q_SIGNALS: // SIGNALS
    void messagesLoaded(uint32_t loadingRequestId,
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
