/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "lrcinstance.h"
#include "qmladapterbase.h"

#include <QObject>
#include <QString>

class MessagesAdapter : public QmlAdapterBase
{
    Q_OBJECT

public:
    explicit MessagesAdapter(QObject *parent = 0);
    ~MessagesAdapter();

    Q_INVOKABLE void setupChatView(const QString &uid);
    Q_INVOKABLE void connectConversationModel();
    Q_INVOKABLE void sendContactRequest();
    Q_INVOKABLE void updateConversationForAddedContact();

    /*
     * JS Q_INVOKABLE.
     */
    Q_INVOKABLE void acceptInvitation(const QString &convUid = "");
    Q_INVOKABLE void refuseInvitation(const QString &convUid = "");
    Q_INVOKABLE void blockConversation(const QString &convUid = "");
    Q_INVOKABLE void setNewMessagesContent(const QString &path);
    Q_INVOKABLE void sendMessage(const QString &message);
    Q_INVOKABLE void sendImage(const QString &message);
    Q_INVOKABLE void sendFile(const QString &message);
    Q_INVOKABLE void retryInteraction(const QString &arg);
    Q_INVOKABLE void deleteInteraction(const QString &arg);
    Q_INVOKABLE void openUrl(const QString &url);
    Q_INVOKABLE void openFile(const QString &arg);
    Q_INVOKABLE void acceptFile(const QString &arg);
    Q_INVOKABLE void refuseFile(const QString &arg);
    Q_INVOKABLE void pasteKeyDetected();
    Q_INVOKABLE void onComposing(bool isComposing);

    /*
     * Manually update draft when hiding message web view (Back to welcome page).
     */
    Q_INVOKABLE void updateDraft();

    /*
     * Run corrsponding js functions, c++ to qml.
     */
    void setMessagesVisibility(bool visible);
    void requestSendMessageContent();
    void setInvitation(bool show, const QString &contactUri = "", const QString &contactId = "");
    void clear();
    void printHistory(lrc::api::ConversationModel &conversationModel,
                      const std::map<uint64_t, lrc::api::interaction::Info> interactions);
    void setSenderImage(const QString &sender, const QString &senderImage);
    void printNewInteraction(lrc::api::ConversationModel &conversationModel,
                             uint64_t msgId,
                             const lrc::api::interaction::Info &interaction);
    void updateInteraction(lrc::api::ConversationModel &conversationModel,
                           uint64_t msgId,
                           const lrc::api::interaction::Info &interaction);
    void setMessagesImageContent(const QString &path, bool isBased64 = false);
    void setMessagesFileContent(const QString &path);
    void removeInteraction(uint64_t interactionId);
    void setSendMessageContent(const QString &content);
    void contactIsComposing(const QString &uid, const QString &contactUri, bool isComposing);

signals:
    void needToUpdateSmartList();

public slots:
    void slotSendMessageContentSaved(const QString &content);
    void slotUpdateDraft(const QString &content);
    void slotMessagesCleared();
    void slotMessagesLoaded();

private:
    void initQmlObject() override final;
    void setConversationProfileData(const lrc::api::conversation::Info &convInfo);
    void newInteraction(const QString &accountId,
                        const QString &convUid,
                        uint64_t interactionId,
                        const interaction::Info &interaction);

    QString LastConvUid_;

    /*
     * Interaction connections.
     */
    QMetaObject::Connection newInteractionConnection_;
    QMetaObject::Connection interactionStatusUpdatedConnection_;
    QMetaObject::Connection interactionRemovedConnection_;
};
