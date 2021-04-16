/*!
 * Copyright (C) 2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "lrcinstance.h"
#include "qmladapterbase.h"
#include "smartlistmodel.h"

#include <QObject>
#include <QString>

class SystemTray;

class ConversationsAdapter final : public QmlAdapterBase
{
    Q_OBJECT

    Q_PROPERTY(lrc::api::profile::Type currentTypeFilter MEMBER currentTypeFilter_ NOTIFY
                   currentTypeFilterChanged)
public:
    explicit ConversationsAdapter(SystemTray* systemTray,
                                  LRCInstance* instance,
                                  QObject* parent = nullptr);
    ~ConversationsAdapter() = default;

protected:
    void safeInit() override;

public:
    Q_INVOKABLE bool connectConversationModel(bool updateFilter = true);
    Q_INVOKABLE void selectConversation(const QString& accountId, const QString& uid);
    Q_INVOKABLE void deselectConversation();
    Q_INVOKABLE void refill();
    Q_INVOKABLE void updateConversationsFilterWidget();

Q_SIGNALS:
    void showConversation(const QString& accountId, const QString& convUid);
    void showConversationTabs(bool visible);
    void showSearchStatus(const QString& status);

    void modelChanged(const QVariant& model);
    void modelSorted(const QVariant& uid);
    void updateListViewRequested();
    void navigateToWelcomePageRequested();
    void currentTypeFilterChanged();
    void indexRepositionRequested();

private Q_SLOTS:
    void onCurrentAccountIdChanged();
    void onNewUnreadInteraction(const QString& accountId,
                                const QString& convUid,
                                uint64_t interactionId,
                                const interaction::Info& interaction);
    void onNewReadInteraction(const QString& accountId,
                              const QString& convUid,
                              uint64_t interactionId);
    void onNewTrustRequest(const QString& accountId, const QString& peerUri);
    void onTrustRequestTreated(const QString& accountId, const QString& peerUri);

private:
    void backToWelcomePage();
    void updateConversationForNewContact(const QString& convUid);

    SmartListModel* conversationSmartListModel_;

    lrc::api::profile::Type currentTypeFilter_ {};

    SystemTray* systemTray_;
};
