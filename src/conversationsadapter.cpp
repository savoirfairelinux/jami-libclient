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

#include <QApplication>

ConversationsAdapter::ConversationsAdapter(QObject* parent, LRCInstance* instance)
    : QmlAdapterBase(parent, instance)
{
    connect(this, &ConversationsAdapter::currentTypeFilterChanged, [this]() {
        lrcInstance_->getCurrentConversationModel()->setFilter(currentTypeFilter_);
    });
}

void
ConversationsAdapter::safeInit()
{
    conversationSmartListModel_ = new SmartListModel(this,
                                                     SmartListModel::Type::CONVERSATION,
                                                     lrcInstance_);

    emit modelChanged(QVariant::fromValue(conversationSmartListModel_));

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::showChatView,
            [this](const QString& accountId, const QString& convId) {
                emit showConversation(accountId, convId);
            });

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::newUnreadInteraction,
            this,
            &ConversationsAdapter::onNewUnreadInteraction);

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
    emit navigateToWelcomePageRequested();
}

void
ConversationsAdapter::selectConversation(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);

    if (lrcInstance_->getCurrentConvUid() != convInfo.uid && convInfo.participants.size() > 0) {
        // If the account is not currently selected, do that first, then
        // proceed to select the conversation.
        auto selectConversation = [this, accountId, convUid = convInfo.uid] {
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
            if (convInfo.uid.isEmpty()) {
                return;
            }
            auto& accInfo = lrcInstance_->getAccountInfo(convInfo.accountId);
            lrcInstance_->setSelectedConvId(convInfo.uid);
            accInfo.conversationModel->clearUnreadInteractions(convInfo.uid);

            // Set contact filter (for conversation tab selection)
            auto& contact = accInfo.contactModel->getContact(convInfo.participants.front());
            setProperty("currentTypeFilter", QVariant::fromValue(contact.profileInfo.type));
        };
        if (convInfo.accountId != lrcInstance_->getCurrAccId()) {
            Utils::oneShotConnect(lrcInstance_,
                                  &LRCInstance::currentAccountChanged,
                                  [selectConversation] { selectConversation(); });
            lrcInstance_->setSelectedConvId();
            lrcInstance_->setSelectedAccountId(convInfo.accountId);
        } else {
            selectConversation();
        }
    }

    if (!convInfo.uid.isEmpty()) {
        emit showConversation(lrcInstance_->getCurrAccId(), convInfo.uid);
    }
}

void
ConversationsAdapter::deselectConversation()
{
    if (lrcInstance_->getCurrentConvUid().isEmpty()) {
        return;
    }

    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    if (currentConversationModel == nullptr) {
        return;
    }

    lrcInstance_->setSelectedConvId();
}

void
ConversationsAdapter::onCurrentAccountIdChanged()
{
    disconnectConversationModel();
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
    Q_UNUSED(interactionId)
    if (!interaction.authorUri.isEmpty()
        && (!QApplication::focusWindow() || accountId != lrcInstance_->getCurrAccId()
            || convUid != lrcInstance_->getCurrentConvUid())) {
        auto& accInfo = lrcInstance_->getAccountInfo(accountId);
        auto from = accInfo.contactModel->bestNameForContact(interaction.authorUri);
        auto onClicked = [this, accountId, convUid, uri = interaction.authorUri] {
            emit lrcInstance_->notificationClicked();
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
            if (!convInfo.uid.isEmpty()) {
                selectConversation(accountId, convInfo.uid);
                emit lrcInstance_->updateSmartList();
                emit modelSorted(convInfo.uid);
            }
        };

        Utils::showNotification(interaction.body, from, accountId, convUid, onClicked);
        return;
    }
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

    modelSortedConnection_ = QObject::connect(
        currentConversationModel, &lrc::api::ConversationModel::modelChanged, [this]() {
            conversationSmartListModel_->fillConversationsList();
            updateConversationsFilterWidget();

            auto* convModel = lrcInstance_->getCurrentConversationModel();
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(
                lrcInstance_->getCurrentConvUid());

            if (convInfo.uid.isEmpty() || convInfo.participants.isEmpty()) {
                return;
            }
            const auto contactURI = convInfo.participants[0];
            if (contactURI.isEmpty()
                || convModel->owner.contactModel->getContact(contactURI).profileInfo.type
                       == lrc::api::profile::Type::TEMPORARY) {
                return;
            }
            emit modelSorted(QVariant::fromValue(convInfo.uid));
        });

    contactProfileUpdatedConnection_
        = QObject::connect(lrcInstance_->getCurrentAccountInfo().contactModel.get(),
                           &lrc::api::ContactModel::profileUpdated,
                           [this](const QString& contactUri) {
                               conversationSmartListModel_->updateContactAvatarUid(contactUri);
                               emit updateListViewRequested();
                           });

    modelUpdatedConnection_ = QObject::connect(currentConversationModel,
                                               &lrc::api::ConversationModel::conversationUpdated,
                                               [this](const QString&) {
                                                   updateConversationsFilterWidget();
                                                   emit updateListViewRequested();
                                               });

    filterChangedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::filterChanged,
                           [this]() {
                               conversationSmartListModel_->fillConversationsList();
                               updateConversationsFilterWidget();
                               if (!lrcInstance_->getCurrentConvUid().isEmpty())
                                   emit indexRepositionRequested();
                               emit updateListViewRequested();
                           });

    newConversationConnection_ = QObject::connect(currentConversationModel,
                                                  &lrc::api::ConversationModel::newConversation,
                                                  [this](const QString& convUid) {
                                                      conversationSmartListModel_
                                                          ->fillConversationsList();
                                                      updateConversationForNewContact(convUid);
                                                  });

    conversationRemovedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::conversationRemoved,
                           [this]() {
                               conversationSmartListModel_->fillConversationsList();
                               backToWelcomePage();
                           });

    conversationClearedConnection
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::conversationCleared,
                           [this](const QString& convUid) {
                               // If currently selected, switch to welcome screen (deselecting
                               // current smartlist item).
                               if (convUid != lrcInstance_->getCurrentConvUid()) {
                                   return;
                               }
                               backToWelcomePage();
                           });

    searchStatusChangedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::searchStatusChanged,
                           [this](const QString& status) { emit showSearchStatus(status); });

    // This connection is ideal when separated search results list.
    // This signal is guaranteed to fire just after filterChanged during a search if results are
    // changed, and once before filterChanged when calling setFilter.
    // NOTE: Currently, when searching, the entire conversation list will be copied 2-3 times each
    // keystroke :/.
    searchResultUpdatedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::searchResultUpdated,
                           [this]() {
                               conversationSmartListModel_->fillConversationsList();
                               emit updateListViewRequested();
                           });

    if (updateFilter) {
        currentTypeFilter_ = lrc::api::profile::Type::INVALID;
    }
    return true;
}

void
ConversationsAdapter::disconnectConversationModel()
{
    QObject::disconnect(modelSortedConnection_);
    QObject::disconnect(modelUpdatedConnection_);
    QObject::disconnect(filterChangedConnection_);
    QObject::disconnect(newConversationConnection_);
    QObject::disconnect(conversationRemovedConnection_);
    QObject::disconnect(conversationClearedConnection);
    QObject::disconnect(selectedCallChanged_);
    QObject::disconnect(smartlistSelectionConnection_);
    QObject::disconnect(interactionRemovedConnection_);
    QObject::disconnect(searchStatusChangedConnection_);
    QObject::disconnect(searchResultUpdatedConnection_);
    QObject::disconnect(contactProfileUpdatedConnection_);
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
                && contact.profileInfo.uri == lrcInstance_->getCurrentConvUid()) {
                lrcInstance_->setSelectedConvId(convUid);
                convModel->selectConversation(convUid);
            }
        } catch (...) {
            return;
        }
    }
}
