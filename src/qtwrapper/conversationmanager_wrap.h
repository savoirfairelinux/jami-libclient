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
#include "conversions_wrap.hpp"

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
        using DRing::exportable_callback;
        using DRing::ConversationSignal;

        conversationsHandlers
            = {exportable_callback<ConversationSignal::ConversationLoaded>(
                   [this](uint32_t loadingRequestId,
                          const std::string& accountId,
                          const std::string& conversationId,
                          const std::vector<std::map<std::string, std::string>>& messages) {
                       Q_EMIT conversationLoaded(loadingRequestId,
                                                 QString(accountId.c_str()),
                                                 QString(conversationId.c_str()),
                                                 convertVecMap(messages));
                   }),
               exportable_callback<ConversationSignal::MessageReceived>(
                   [this](const std::string& accountId,
                          const std::string& conversationId,
                          const std::map<std::string, std::string>& message) {
                       Q_EMIT messageReceived(QString(accountId.c_str()),
                                              QString(conversationId.c_str()),
                                              convertMap(message));
                   }),
               exportable_callback<ConversationSignal::ConversationRequestReceived>(
                   [this](const std::string& accountId,
                          const std::string& conversationId,
                          const std::map<std::string, std::string>& metadata) {
                       Q_EMIT conversationRequestReceived(QString(accountId.c_str()),
                                                          QString(conversationId.c_str()),
                                                          convertMap(metadata));
                   }),
               exportable_callback<ConversationSignal::ConversationReady>(
                   [this](const std::string& accountId, const std::string& conversationId) {
                       Q_EMIT conversationReady(QString(accountId.c_str()),
                                                QString(conversationId.c_str()));
                   })};
    }

    ~ConversationManagerInterface() {}
    bool isValid() { return true; }

public Q_SLOTS: // METHODS
    void startConversation(const QString& accountId)
    {
        DRing::startConversation(accountId.toStdString());
    }
    void acceptConversationRequest(const QString& accountId, const QString& conversationId)
    {
        DRing::acceptConversationRequest(accountId.toStdString(), conversationId.toStdString());
    }
    void declineConversationRequest(const QString& accountId, const QString& conversationId)
    {
        DRing::declineConversationRequest(accountId.toStdString(), conversationId.toStdString());
    }
    bool removeConversation(const QString& accountId, const QString& conversationId)
    {
        return DRing::removeConversation(accountId.toStdString(), conversationId.toStdString());
    }
    std::vector<std::string> getConversations(const QString& accountId)
    {
        return DRing::getConversations(accountId.toStdString());
    }
    std::vector<std::map<std::string, std::string>> getConversationRequests(const QString& accountId)
    {
        return DRing::getConversationRequests(accountId.toStdString());
    }
    bool addConversationMember(const QString& accountId,
                               const QString& conversationId,
                               const QString& memberURI)
    {
        return DRing::addConversationMember(accountId.toStdString(),
                                            conversationId.toStdString(),
                                            memberURI.toStdString());
    }
    bool removeConversationMember(const QString& accountId,
                                  const QString& conversationId,
                                  const QString& memberURI)
    {
        return DRing::removeConversationMember(accountId.toStdString(),
                                               conversationId.toStdString(),
                                               memberURI.toStdString());
    }
    std::vector<std::map<std::string, std::string>> getConversationMembers(
        const QString& accountId, const QString& conversationId)
    {
        return DRing::getConversationMembers(accountId.toStdString(), conversationId.toStdString());
    }
    void sendMessage(const QString& accountId,
                     const QString& conversationId,
                     const QString& message,
                     const QString& parrent)
    {
        DRing::sendMessage(accountId.toStdString(),
                           conversationId.toStdString(),
                           message.toStdString(),
                           parrent.toStdString());
    }
    uint32_t loadConversationMessages(const QString& accountId,
                                      const QString& conversationId,
                                      const QString& fromId,
                                      const int size)
    {
        return DRing::loadConversationMessages(accountId.toStdString(),
                                               conversationId.toStdString(),
                                               fromId.toStdString(),
                                               size);
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
