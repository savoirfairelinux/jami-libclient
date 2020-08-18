/*
 * Copyright (C) 2017-2020 by Savoir-faire Linux
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/conversationmodel.h"

#include <QAbstractItemModel>

using namespace lrc::api;

class SmartListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    using AccountInfo = lrc::api::account::Info;
    using ConversationInfo = lrc::api::conversation::Info;
    using ContactInfo = lrc::api::contact::Info;

    enum class Type { CONVERSATION, CONFERENCE, TRANSFER, COUNT__ };

    enum Role {
        DisplayName = Qt::UserRole + 1,
        DisplayID,
        Picture,
        Presence,
        URI,
        UnreadMessagesCount,
        LastInteractionDate,
        LastInteraction,
        LastInteractionType,
        ContactType,
        UID,
        ContextMenuOpen,
        InCall,
        IsAudioOnly,
        CallStackViewShouldShow,
        CallStateStr,
        SectionName,
        AccountId,
        Draft
    };

    explicit SmartListModel(const QString &accId,
                            QObject *parent = 0,
                            SmartListModel::Type listModelType = Type::CONVERSATION,
                            const QString &convUid = {});
    ~SmartListModel();

    /*
     * QAbstractListModel.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    Q_INVOKABLE void setAccount(const QString &accId);
    Q_INVOKABLE void setConferenceableFilter(const QString &filter = {});
    Q_INVOKABLE void toggleSection(const QString &section);
    Q_INVOKABLE int currentUidSmartListModelIndex();
    Q_INVOKABLE void fillConversationsList();
    Q_INVOKABLE void updateConversation(const QString &conv);

private:
    QString accountId_;

    QVariant getConversationItemData(const ConversationInfo &item,
                                     const AccountInfo &accountInfo,
                                     int role) const;
    /*
     * List sectioning.
     */
    QString convUid_;
    Type listModelType_;
    QMap<QString, bool> sectionState_;
    QMap<ConferenceableItem, ConferenceableValue> conferenceables_;
    ConversationModel::ConversationQueue conversations_;
};
