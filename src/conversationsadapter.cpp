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

#include "conversationsadapter.h"

#include "utils.h"
#include "qtutils.h"
#include "systemtray.h"
#include "qmlregister.h"

#include <QApplication>
#include <QJsonObject>

using namespace lrc::api;

ConversationsAdapter::ConversationsAdapter(SystemTray* systemTray,
                                           LRCInstance* instance,
                                           QObject* parent)
    : QmlAdapterBase(instance, parent)
    , currentTypeFilter_(profile::Type::JAMI)
    , systemTray_(systemTray)
    , convSrcModel_(new ConversationListModel(lrcInstance_))
    , convModel_(new ConversationListProxyModel(convSrcModel_.get()))
    , searchSrcModel_(new SearchResultsListModel(lrcInstance_))
    , searchModel_(new SelectableListProxyModel(searchSrcModel_.get()))
{
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_MODELS, convModel_.get(), "ConversationListModel");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_MODELS, searchModel_.get(), "SearchResultsListModel");

    new SelectableListProxyGroupModel({convModel_.data(), searchModel_.data()}, this);

    setTypeFilter(currentTypeFilter_);
    connect(this, &ConversationsAdapter::currentTypeFilterChanged, [this]() {
        setTypeFilter(currentTypeFilter_);
    });

    connect(lrcInstance_, &LRCInstance::selectedConvUidChanged, [this]() {
        auto convId = lrcInstance_->get_selectedConvUid();
        if (convId.isEmpty()) {
            // deselected
            convModel_->deselect();
            searchModel_->deselect();
            Q_EMIT navigateToWelcomePageRequested();
        } else {
            // selected
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(convId);
            if (convInfo.uid.isEmpty())
                return;

            auto& accInfo = lrcInstance_->getAccountInfo(convInfo.accountId);
            accInfo.conversationModel->selectConversation(convInfo.uid);
            accInfo.conversationModel->clearUnreadInteractions(convInfo.uid);

            try {
                // Set contact filter (for conversation tab selection)
                // WARNING: not swarm ready
                auto& contact = accInfo.contactModel->getContact(convInfo.participants.front());
                if (contact.profileInfo.type != profile::Type::INVALID
                    && contact.profileInfo.type != profile::Type::TEMPORARY)
                    set_currentTypeFilter(contact.profileInfo.type);
            } catch (const std::out_of_range& e) {
                qWarning() << e.what();
            }

            // reposition index in case of programmatic selection
            // currently, this may only occur for the conversation list
            // and not the search list
            convModel_->selectSourceRow(lrcInstance_->indexOf(convId));
        }
    });

    connect(lrcInstance_, &LRCInstance::draftSaved, [this](const QString& convId) {
        auto row = lrcInstance_->indexOf(convId);
        const auto index = convSrcModel_->index(row, 0);
        Q_EMIT convSrcModel_->dataChanged(index, index);
    });

    connect(lrcInstance_, &LRCInstance::contactBanned, [this](const QString& uri) {
        auto& convInfo = lrcInstance_->getConversationFromPeerUri(uri);
        if (convInfo.uid.isEmpty())
            return;
        auto row = lrcInstance_->indexOf(convInfo.uid);
        const auto index = convSrcModel_->index(row, 0);
        Q_EMIT convSrcModel_->dataChanged(index, index);
        lrcInstance_->set_selectedConvUid();
    });

#ifdef Q_OS_LINUX
    // notification responses
    connect(systemTray_,
            &SystemTray::openConversationActivated,
            [this](const QString& accountId, const QString& convUid) {
                Q_EMIT lrcInstance_->notificationClicked();
                lrcInstance_->selectConversation(convUid, accountId);
            });
    connect(systemTray_,
            &SystemTray::acceptPendingActivated,
            [this](const QString& accountId, const QString& peerUri) {
                auto& convInfo = lrcInstance_->getConversationFromPeerUri(peerUri, accountId);
                if (convInfo.uid.isEmpty())
                    return;
                lrcInstance_->getAccountInfo(accountId).conversationModel->makePermanent(
                    convInfo.uid);
            });
    connect(systemTray_,
            &SystemTray::refusePendingActivated,
            [this](const QString& accountId, const QString& peerUri) {
                auto& convInfo = lrcInstance_->getConversationFromPeerUri(peerUri, accountId);
                if (convInfo.uid.isEmpty())
                    return;
                lrcInstance_->getAccountInfo(accountId).conversationModel->removeConversation(
                    convInfo.uid);
            });
#endif
}

void
ConversationsAdapter::safeInit()
{
    // TODO: remove these safeInits, they are possibly called
    // multiple times during qml component inits
    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newUnreadInteraction,
            this,
            &ConversationsAdapter::onNewUnreadInteraction,
            Qt::UniqueConnection);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newReadInteraction,
            this,
            &ConversationsAdapter::onNewReadInteraction,
            Qt::UniqueConnection);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newTrustRequest,
            this,
            &ConversationsAdapter::onNewTrustRequest,
            Qt::UniqueConnection);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::trustRequestTreated,
            this,
            &ConversationsAdapter::onTrustRequestTreated,
            Qt::UniqueConnection);

    connect(lrcInstance_,
            &LRCInstance::currentAccountIdChanged,
            this,
            &ConversationsAdapter::onCurrentAccountIdChanged,
            Qt::UniqueConnection);

    connectConversationModel();

    set_currentTypeFilter(lrcInstance_->getCurrentAccountInfo().profileInfo.type);
}

void
ConversationsAdapter::onCurrentAccountIdChanged()
{
    lrcInstance_->deselectConversation();

    connectConversationModel();

    set_currentTypeFilter(lrcInstance_->getCurrentAccountInfo().profileInfo.type);
}

void
ConversationsAdapter::onNewUnreadInteraction(const QString& accountId,
                                             const QString& convUid,
                                             const QString& interactionId,
                                             const interaction::Info& interaction)
{
    if (!interaction.authorUri.isEmpty()
        && (!QApplication::focusWindow() || accountId != lrcInstance_->getCurrentAccountId()
            || convUid != lrcInstance_->get_selectedConvUid())) {
        auto& accountInfo = lrcInstance_->getAccountInfo(accountId);
        auto from = accountInfo.contactModel->bestNameForContact(interaction.authorUri);
#ifdef Q_OS_LINUX
        auto contactPhoto = Utils::contactPhoto(lrcInstance_,
                                                interaction.authorUri,
                                                QSize(50, 50),
                                                accountId);
        auto notifId = QString("%1;%2;%3").arg(accountId).arg(convUid).arg(interactionId);
        systemTray_->showNotification(notifId,
                                      tr("New message"),
                                      from + ": " + interaction.body,
                                      NotificationType::CHAT,
                                      Utils::QImageToByteArray(contactPhoto));

#else
        Q_UNUSED(interactionId)
        auto onClicked = [this, accountId, convUid, uri = interaction.authorUri] {
            Q_EMIT lrcInstance_->notificationClicked();
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
            if (convInfo.uid.isEmpty())
                return;
            lrcInstance_->selectConversation(convInfo.uid, accountId);
        };
        systemTray_->showNotification(interaction.body, from, onClicked);
#endif
    }
}

void
ConversationsAdapter::onNewReadInteraction(const QString& accountId,
                                           const QString& convUid,
                                           const QString& interactionId)
{
#ifdef Q_OS_LINUX
    // hide notification
    auto notifId = QString("%1;%2;%3").arg(accountId).arg(convUid).arg(interactionId);
    systemTray_->hideNotification(notifId);
#else
    Q_UNUSED(accountId)
    Q_UNUSED(convUid)
    Q_UNUSED(interactionId)
#endif
}

void
ConversationsAdapter::onNewTrustRequest(const QString& accountId, const QString& peerUri)
{
#ifdef Q_OS_LINUX
    if (!QApplication::focusWindow() || accountId != lrcInstance_->getCurrentAccountId()) {
        auto& accInfo = lrcInstance_->getAccountInfo(accountId);
        auto from = accInfo.contactModel->bestNameForContact(peerUri);
        auto contactPhoto = Utils::contactPhoto(lrcInstance_, peerUri, QSize(50, 50), accountId);
        auto notifId = QString("%1;%2").arg(accountId).arg(peerUri);
        systemTray_->showNotification(notifId,
                                      tr("Trust request"),
                                      "New request from " + from,
                                      NotificationType::REQUEST,
                                      Utils::QImageToByteArray(contactPhoto));
    }
#else
    Q_UNUSED(accountId)
    Q_UNUSED(peerUri)
#endif
}

void
ConversationsAdapter::onTrustRequestTreated(const QString& accountId, const QString& peerUri)
{
#ifdef Q_OS_LINUX
    // hide notification
    auto notifId = QString("%1;%2").arg(accountId).arg(peerUri);
    systemTray_->hideNotification(notifId);
#else
    Q_UNUSED(accountId)
    Q_UNUSED(peerUri)
#endif
}

void
ConversationsAdapter::onModelChanged()
{
    updateConversationFilterData();
}

void
ConversationsAdapter::onProfileUpdated(const QString& contactUri)
{
    auto& convInfo = lrcInstance_->getConversationFromPeerUri(contactUri);
    if (convInfo.uid.isEmpty())
        return;
    auto row = lrcInstance_->indexOf(convInfo.uid);
    const auto index = convSrcModel_->index(row, 0);

    convSrcModel_->updateContactAvatarUid(contactUri);
    Q_EMIT convSrcModel_->dataChanged(index, index);
}

void
ConversationsAdapter::onConversationUpdated(const QString&)
{
    updateConversationFilterData();
}

void
ConversationsAdapter::onFilterChanged()
{
    updateConversationFilterData();
    if (!lrcInstance_->get_selectedConvUid().isEmpty())
        Q_EMIT indexRepositionRequested();
}

void
ConversationsAdapter::onNewConversation(const QString& convUid)
{
    updateConversationForNewContact(convUid);
}

void
ConversationsAdapter::onConversationCleared(const QString& convUid)
{
    // If currently selected, switch to welcome screen (deselecting
    // current smartlist item).
    if (convUid == lrcInstance_->get_selectedConvUid()) {
        lrcInstance_->deselectConversation();
    }
}

void
ConversationsAdapter::onSearchStatusChanged(const QString& status)
{
    Q_EMIT showSearchStatus(status);
}

void
ConversationsAdapter::onSearchResultUpdated()
{
    // smartlist search results
    searchSrcModel_->onSearchResultsUpdated();
}

void
ConversationsAdapter::updateConversationFilterData()
{
    // TODO: this may be further spliced to respond separately to
    // incoming messages and invites
    // total unread message and pending invite counts, and tab selection
    auto& accountInfo = lrcInstance_->getCurrentAccountInfo();
    int totalUnreadMessages {0};
    if (accountInfo.profileInfo.type != profile::Type::SIP) {
        auto& convModel = accountInfo.conversationModel;
        auto conversations = convModel->getFilteredConversations(FilterType::JAMI, false);
        conversations.for_each([&totalUnreadMessages](const conversation::Info& conversation) {
            totalUnreadMessages += conversation.unreadMessages;
        });
    }
    set_totalUnreadMessageCount(totalUnreadMessages);
    set_pendingRequestCount(accountInfo.conversationModel->pendingRequestCount());
    if (pendingRequestCount_ == 0 && currentTypeFilter_ == profile::Type::PENDING) {
        set_currentTypeFilter(profile::Type::JAMI);
    }
}

void
ConversationsAdapter::setFilter(const QString& filterString)
{
    convModel_->setFilter(filterString);
    searchSrcModel_->setFilter(filterString);
}

void
ConversationsAdapter::setTypeFilter(const profile::Type& typeFilter)
{
    convModel_->setTypeFilter(typeFilter);
}

QVariantMap
ConversationsAdapter::getConvInfoMap(const QString& convId)
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convId);
    if (convInfo.participants.empty())
        return {};
    auto peerUri = convInfo.participants[0];
    ContactModel* contactModel {nullptr};
    contact::Info contact {};
    try {
        const auto& accountInfo = lrcInstance_->getAccountInfo(convInfo.accountId);
        contactModel = accountInfo.contactModel.get();
        contact = contactModel->getContact(peerUri);
    } catch (...) {
        return {};
    }
    bool isAudioOnly {false};
    if (!convInfo.uid.isEmpty()) {
        auto* call = lrcInstance_->getCallInfoForConversation(convInfo);
        if (call) {
            isAudioOnly = call->isAudioOnly;
        }
    }
    bool callStackViewShouldShow {false};
    call::Status callState {};
    if (!convInfo.callId.isEmpty()) {
        auto* callModel = lrcInstance_->getCurrentCallModel();
        const auto& call = callModel->getCall(convInfo.callId);
        callStackViewShouldShow = callModel->hasCall(convInfo.callId)
                                  && ((!call.isOutgoing
                                       && (call.status == call::Status::IN_PROGRESS
                                           || call.status == call::Status::PAUSED
                                           || call.status == call::Status::INCOMING_RINGING))
                                      || (call.isOutgoing && call.status != call::Status::ENDED));
        callState = call.status;
    }
    // WARNING: not swarm ready
    // titles should come from conversation, not contact model
    return {{"convId", convId},
            {"bestId", contactModel->bestIdForContact(peerUri)},
            {"bestName", contactModel->bestNameForContact(peerUri)},
            {"uri", peerUri},
            {"isSwarm", !convInfo.isNotASwarm()},
            {"contactType", static_cast<int>(contact.profileInfo.type)},
            {"isAudioOnly", isAudioOnly},
            {"callState", static_cast<int>(callState)},
            {"callStackViewShouldShow", callStackViewShouldShow}};
}

bool
ConversationsAdapter::connectConversationModel(bool updateFilter)
{
    // Signal connections
    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::modelChanged,
                     this,
                     &ConversationsAdapter::onModelChanged,
                     Qt::UniqueConnection);

    QObject::connect(lrcInstance_->getCurrentAccountInfo().contactModel.get(),
                     &lrc::api::ContactModel::profileUpdated,
                     this,
                     &ConversationsAdapter::onProfileUpdated,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::conversationUpdated,
                     this,
                     &ConversationsAdapter::onConversationUpdated,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::filterChanged,
                     this,
                     &ConversationsAdapter::onFilterChanged,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::newConversation,
                     this,
                     &ConversationsAdapter::onNewConversation,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::conversationCleared,
                     this,
                     &ConversationsAdapter::onConversationCleared,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::searchStatusChanged,
                     this,
                     &ConversationsAdapter::onSearchStatusChanged,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &lrc::api::ConversationModel::searchResultUpdated,
                     this,
                     &ConversationsAdapter::onSearchResultUpdated,
                     Qt::UniqueConnection);

    if (updateFilter) {
        currentTypeFilter_ = profile::Type::INVALID;
    }

    convSrcModel_.reset(new ConversationListModel(lrcInstance_));
    convModel_->bindSourceModel(convSrcModel_.get());
    searchSrcModel_.reset(new SearchResultsListModel(lrcInstance_));
    searchModel_->bindSourceModel(searchSrcModel_.get());

    updateConversationFilterData();

    return true;
}

void
ConversationsAdapter::updateConversationForNewContact(const QString& convUid)
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    if (convModel == nullptr) {
        return;
    }
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid);

    if (!convInfo.uid.isEmpty() && !convInfo.participants.isEmpty()) {
        try {
            const auto contact = convModel->owner.contactModel->getContact(convInfo.participants[0]);
            if (!contact.profileInfo.uri.isEmpty()
                && contact.profileInfo.uri == lrcInstance_->get_selectedConvUid()) {
                lrcInstance_->selectConversation(convUid, convInfo.accountId);
                convModel_->selectSourceRow(lrcInstance_->indexOf(convUid));
            }
        } catch (...) {
            return;
        }
    }
}
