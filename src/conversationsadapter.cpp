/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
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

#include "conversationsadapter.h"

#include "utils.h"
#include "qtutils.h"
#include "systemtray.h"

#include <QApplication>

ConversationsAdapter::ConversationsAdapter(SystemTray* systemTray,
                                           LRCInstance* instance,
                                           QObject* parent)
    : QmlAdapterBase(instance, parent)
    , systemTray_(systemTray)
{
    connect(this, &ConversationsAdapter::currentTypeFilterChanged, [this]() {
        lrcInstance_->getCurrentConversationModel()->setFilter(currentTypeFilter_);
    });

    connect(lrcInstance_, &LRCInstance::conversationSelected, [this]() {
        auto convUid = lrcInstance_->get_selectedConvUid();
        if (!convUid.isEmpty()) {
            Q_EMIT showConversation(lrcInstance_->getCurrAccId(), convUid);
        }
    });

#ifdef Q_OS_LINUX
    // notification responses
    connect(systemTray_,
            &SystemTray::openConversationActivated,
            [this](const QString& accountId, const QString& convUid) {
                Q_EMIT lrcInstance_->notificationClicked();
                selectConversation(accountId, convUid);
                Q_EMIT lrcInstance_->updateSmartList();
                Q_EMIT modelSorted(convUid);
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
    conversationSmartListModel_ = new SmartListModel(this,
                                                     SmartListModel::Type::CONVERSATION,
                                                     lrcInstance_);

    Q_EMIT modelChanged(QVariant::fromValue(conversationSmartListModel_));

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::showChatView,
            [this](const QString& accountId, const QString& convId) {
                Q_EMIT showConversation(accountId, convId);
            });

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newUnreadInteraction,
            this,
            &ConversationsAdapter::onNewUnreadInteraction);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newReadInteraction,
            this,
            &ConversationsAdapter::onNewReadInteraction);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newTrustRequest,
            this,
            &ConversationsAdapter::onNewTrustRequest);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::trustRequestTreated,
            this,
            &ConversationsAdapter::onTrustRequestTreated);

    connect(lrcInstance_,
            &LRCInstance::currentAccountChanged,
            this,
            &ConversationsAdapter::onCurrentAccountIdChanged);

    connectConversationModel();

    setProperty("currentTypeFilter",
                QVariant::fromValue(lrcInstance_->getCurrentAccountInfo().profileInfo.type));
}

void
ConversationsAdapter::backToWelcomePage()
{
    deselectConversation();
    Q_EMIT navigateToWelcomePageRequested();
}

void
ConversationsAdapter::selectConversation(const QString& accountId, const QString& convUid)
{
    lrcInstance_->selectConversation(accountId, convUid);
}

void
ConversationsAdapter::deselectConversation()
{
    if (lrcInstance_->get_selectedConvUid().isEmpty()) {
        return;
    }

    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    if (currentConversationModel == nullptr) {
        return;
    }

    lrcInstance_->set_selectedConvUid();
}

void
ConversationsAdapter::onCurrentAccountIdChanged()
{
    connectConversationModel();

    setProperty("currentTypeFilter",
                QVariant::fromValue(lrcInstance_->getCurrentAccountInfo().profileInfo.type));
}

void
ConversationsAdapter::onNewUnreadInteraction(const QString& accountId,
                                             const QString& convUid,
                                             uint64_t interactionId,
                                             const interaction::Info& interaction)
{
    if (!interaction.authorUri.isEmpty()
        && (!QApplication::focusWindow() || accountId != lrcInstance_->getCurrAccId()
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
            if (!convInfo.uid.isEmpty()) {
                selectConversation(accountId, convInfo.uid);
                Q_EMIT lrcInstance_->updateSmartList();
                Q_EMIT modelSorted(convInfo.uid);
            }
        };
        systemTray_->showNotification(interaction.body, from, onClicked);
#endif
    }
}

void
ConversationsAdapter::onNewReadInteraction(const QString& accountId,
                                           const QString& convUid,
                                           uint64_t interactionId)
{
#ifdef Q_OS_LINUX
    // hide notification
    auto notifId = QString("%1;%2;%3").arg(accountId).arg(convUid).arg(interactionId);
    systemTray_->hideNotification(notifId);
#endif
}

void
ConversationsAdapter::onNewTrustRequest(const QString& accountId, const QString& peerUri)
{
#ifdef Q_OS_LINUX
    if (!QApplication::focusWindow() || accountId != lrcInstance_->getCurrAccId()) {
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
#endif
}

void
ConversationsAdapter::onTrustRequestTreated(const QString& accountId, const QString& peerUri)
{
#ifdef Q_OS_LINUX
    // hide notification
    auto notifId = QString("%1;%2").arg(accountId).arg(peerUri);
    systemTray_->hideNotification(notifId);
#endif
}

void
ConversationsAdapter::updateConversationsFilterWidget()
{
    // Update status of "Conversations" and "Invitations".
    auto invites = lrcInstance_->getCurrentAccountInfo().contactModel->pendingRequestCount();
    if (invites == 0 && currentTypeFilter_ == lrc::api::profile::Type::PENDING) {
        setProperty("currentTypeFilter", QVariant::fromValue(lrc::api::profile::Type::RING));
    }
    showConversationTabs(invites);
}

void
ConversationsAdapter::refill()
{
    if (conversationSmartListModel_)
        conversationSmartListModel_->fillConversationsList();
}

bool
ConversationsAdapter::connectConversationModel(bool updateFilter)
{
    // Signal connections
    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::modelChanged,
        this,
        [this]() {
            conversationSmartListModel_->fillConversationsList();
            updateConversationsFilterWidget();

            auto* convModel = lrcInstance_->getCurrentConversationModel();
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(
                lrcInstance_->get_selectedConvUid());

            if (convInfo.uid.isEmpty() || convInfo.participants.isEmpty()) {
                return;
            }
            const auto contactURI = convInfo.participants[0];
            if (contactURI.isEmpty()
                || convModel->owner.contactModel->getContact(contactURI).profileInfo.type
                       == lrc::api::profile::Type::TEMPORARY) {
                return;
            }
            Q_EMIT modelSorted(QVariant::fromValue(convInfo.uid));
        },
        Qt::UniqueConnection);

    QObject::connect(
        lrcInstance_->getCurrentAccountInfo().contactModel.get(),
        &lrc::api::ContactModel::profileUpdated,
        this,
        [this](const QString& contactUri) {
            conversationSmartListModel_->updateContactAvatarUid(contactUri);
            Q_EMIT updateListViewRequested();
        },
        Qt::UniqueConnection);

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::conversationUpdated,
        this,
        [this](const QString&) {
            updateConversationsFilterWidget();
            Q_EMIT updateListViewRequested();
        },
        Qt::UniqueConnection);

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::filterChanged,
        this,
        [this]() {
            conversationSmartListModel_->fillConversationsList();
            updateConversationsFilterWidget();
            if (!lrcInstance_->get_selectedConvUid().isEmpty())
                Q_EMIT indexRepositionRequested();
            Q_EMIT updateListViewRequested();
        },
        Qt::UniqueConnection);

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::newConversation,
        this,
        [this](const QString& convUid) {
            conversationSmartListModel_->fillConversationsList();
            updateConversationForNewContact(convUid);
        },
        Qt::UniqueConnection);

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::conversationRemoved,
        this,
        [this]() { backToWelcomePage(); },
        Qt::UniqueConnection);

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::conversationCleared,
        this,
        [this](const QString& convUid) {
            // If currently selected, switch to welcome screen (deselecting
            // current smartlist item).
            if (convUid != lrcInstance_->get_selectedConvUid()) {
                return;
            }
            backToWelcomePage();
        },
        Qt::UniqueConnection);

    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::searchStatusChanged,
        this,
        [this](const QString& status) { Q_EMIT showSearchStatus(status); },
        Qt::UniqueConnection);

    // This connection is ideal when separated search results list.
    // This signal is guaranteed to fire just after filterChanged during a search if results are
    // changed, and once before filterChanged when calling setFilter.
    // NOTE: Currently, when searching, the entire conversation list will be copied 2-3 times each
    // keystroke :/.
    QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::searchResultUpdated,
        this,
        [this]() {
            conversationSmartListModel_->fillConversationsList();
            Q_EMIT updateListViewRequested();
        },
        Qt::UniqueConnection);

    if (updateFilter) {
        currentTypeFilter_ = lrc::api::profile::Type::INVALID;
    }
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
                lrcInstance_->set_selectedConvUid(convUid);
                convModel->selectConversation(convUid);
            }
        } catch (...) {
            return;
        }
    }
}
