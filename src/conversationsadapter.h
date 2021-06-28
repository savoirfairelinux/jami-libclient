/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
#include "smartlistmodel.h"
#include "conversationlistmodel.h"
#include "searchresultslistmodel.h"

#include <QObject>
#include <QString>

class SystemTray;

class ConversationsAdapter final : public QmlAdapterBase
{
    Q_OBJECT
    QML_PROPERTY(bool, filterRequests)
    QML_PROPERTY(int, totalUnreadMessageCount)
    QML_PROPERTY(int, pendingRequestCount)

public:
    explicit ConversationsAdapter(SystemTray* systemTray,
                                  LRCInstance* instance,
                                  QObject* parent = nullptr);
    ~ConversationsAdapter() = default;

protected:
    void safeInit() override;

public:
    Q_INVOKABLE bool connectConversationModel();
    Q_INVOKABLE void setFilter(const QString& filterString);
    Q_INVOKABLE QVariantMap getConvInfoMap(const QString& convId);

Q_SIGNALS:
    void showConversation(const QString& accountId, const QString& convUid);
    void showSearchStatus(const QString& status);

    void navigateToWelcomePageRequested();
    void indexRepositionRequested();
    void conversationReady(const QString& convId);

private Q_SLOTS:
    void onCurrentAccountIdChanged();

    // cross-account slots
    void onNewUnreadInteraction(const QString& accountId,
                                const QString& convUid,
                                const QString& interactionId,
                                const interaction::Info& interaction);
    void onNewReadInteraction(const QString& accountId,
                              const QString& convUid,
                              const QString& interactionId);
    void onNewTrustRequest(const QString& accountId, const QString& peerUri);
    void onTrustRequestTreated(const QString& accountId, const QString& peerUri);

    // per-account slots
    void onModelChanged();
    void onProfileUpdated(const QString&);
    void onConversationUpdated(const QString&);
    void onFilterChanged();
    void onConversationCleared(const QString&);
    void onSearchStatusChanged(const QString&);
    void onSearchResultUpdated();
    void updateConversation(const QString&);

    void updateConversationFilterData();

private:
    void updateConversationForNewContact(const QString& convUid);

    SystemTray* systemTray_;

    QScopedPointer<ConversationListModel> convSrcModel_;
    QScopedPointer<ConversationListProxyModel> convModel_;
    QScopedPointer<SearchResultsListModel> searchSrcModel_;
    QScopedPointer<SelectableListProxyModel> searchModel_;
};
