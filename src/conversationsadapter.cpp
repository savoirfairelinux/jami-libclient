/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony L�onard <anthony.leonard@savoirfairelinux.com>
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

ConversationsAdapter::ConversationsAdapter(QObject* parent)
    : QmlAdapterBase(parent)
{
    connect(this, &ConversationsAdapter::currentTypeFilterChanged, [this]() {
        LRCInstance::getCurrentConversationModel()->setFilter(currentTypeFilter_);
    });
}

void
ConversationsAdapter::safeInit()
{
    conversationSmartListModel_ = new SmartListModel(this, LRCInstance::getCurrAccId());

    emit modelChanged(QVariant::fromValue(conversationSmartListModel_));

    connect(&LRCInstance::behaviorController(),
            &BehaviorController::showChatView,
            [this](const QString& accountId, lrc::api::conversation::Info convInfo) {
                emit showConversation(accountId, convInfo.uid);
            });

    connect(&LRCInstance::behaviorController(),
            &BehaviorController::newUnreadInteraction,
            this,
            &ConversationsAdapter::onNewUnreadInteraction);

    connect(&LRCInstance::instance(),
            &LRCInstance::currentAccountChanged,
            this,
            &ConversationsAdapter::onCurrentAccountIdChanged);

    connectConversationModel();

    setProperty("currentTypeFilter", QVariant::fromValue(
                    LRCInstance::getCurrentAccountInfo().profileInfo.type));

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
    auto* convModel = LRCInstance::getAccountInfo(accountId).conversationModel.get();
    const auto& convInfo = convModel->getConversationForUID(convUid);

    if (LRCInstance::getCurrentConvUid() != convInfo.uid && convInfo.participants.size() > 0) {
        // If the account is not currently selected, do that first, then
        // proceed to select the conversation.
        auto selectConversation = [this, convInfo] {
            auto& accInfo = LRCInstance::getAccountInfo(convInfo.accountId);
            LRCInstance::setSelectedConvId(convInfo.uid);
            accInfo.conversationModel->clearUnreadInteractions(convInfo.uid);

            // Set contact filter (for conversation tab selection)
            auto& contact = accInfo.contactModel->getContact(convInfo.participants.front());
            setProperty("currentTypeFilter", QVariant::fromValue(contact.profileInfo.type));
        };
        if (convInfo.accountId != LRCInstance::getCurrAccId()) {
            Utils::oneShotConnect(&LRCInstance::instance(),
                                  &LRCInstance::currentAccountChanged,
                                  [selectConversation] { selectConversation(); });
            LRCInstance::setSelectedConvId(); // Hack UI
            LRCInstance::setSelectedAccountId(convInfo.accountId);
        } else {
            selectConversation();
        }
    }

    if (!convInfo.uid.isEmpty()) {
        emit showConversation(LRCInstance::getCurrAccId(), convInfo.uid);
    }
}

void
ConversationsAdapter::deselectConversation()
{
    if (LRCInstance::getCurrentConvUid().isEmpty()) {
        return;
    }

    auto currentConversationModel = LRCInstance::getCurrentConversationModel();

    if (currentConversationModel == nullptr) {
        return;
    }

    LRCInstance::setSelectedConvId();
}

void
ConversationsAdapter::onCurrentAccountIdChanged()
{
    disconnectConversationModel();
    connectConversationModel();

    setProperty("currentTypeFilter", QVariant::fromValue(
                    LRCInstance::getCurrentAccountInfo().profileInfo.type));
}

void
ConversationsAdapter::onNewUnreadInteraction(const QString& accountId,
                                             const QString& convUid,
                                             uint64_t interactionId,
                                             const interaction::Info& interaction)
{
    Q_UNUSED(interactionId)
    if (!interaction.authorUri.isEmpty()
        && (!QApplication::focusWindow() || accountId != LRCInstance::getCurrAccId()
            || convUid != LRCInstance::getCurrentConvUid())) {
        auto& accInfo = LRCInstance::getAccountInfo(accountId);
        auto& contact = accInfo.contactModel->getContact(interaction.authorUri);
        auto from = Utils::bestNameForContact(contact);
        auto onClicked = [this,
                          accountId,
                          convUid,
                          uri = interaction.authorUri] {
#ifdef Q_OS_WINDOWS
            emit LRCInstance::instance().notificationClicked();
#else
            emit LRCInstance::instance().notificationClicked(true);
#endif
            auto convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
            if (!convInfo.uid.isEmpty()) {
                selectConversation(accountId, convInfo.uid);
                emit LRCInstance::instance().updateSmartList();
                emit modelSorted(uri);
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
    auto invites = LRCInstance::getCurrentAccountInfo().contactModel->pendingRequestCount();
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
    auto currentConversationModel = LRCInstance::getCurrentConversationModel();

    modelSortedConnection_ = QObject::connect(
        currentConversationModel, &lrc::api::ConversationModel::modelSorted, [this]() {
            conversationSmartListModel_->fillConversationsList();
            updateConversationsFilterWidget();
            emit updateListViewRequested();
            auto* convModel = LRCInstance::getCurrentConversationModel();
            const auto conversation = convModel->getConversationForUID(
                LRCInstance::getCurrentConvUid());

            if (conversation.uid.isEmpty() || conversation.participants.isEmpty()) {
                return;
            }
            const auto contactURI = conversation.participants[0];
            if (contactURI.isEmpty()
                || convModel->owner.contactModel->getContact(contactURI).profileInfo.type
                       == lrc::api::profile::Type::TEMPORARY) {
                return;
            }
            emit modelSorted(QVariant::fromValue(contactURI));
        });

    modelUpdatedConnection_ = QObject::connect(currentConversationModel,
                                               &lrc::api::ConversationModel::conversationUpdated,
                                               [this](const QString& convUid) {
                                                   conversationSmartListModel_->updateConversation(
                                                       convUid);
                                                   updateConversationsFilterWidget();
                                                   emit updateListViewRequested();
                                               });

    filterChangedConnection_ = QObject::connect(currentConversationModel,
                                                &lrc::api::ConversationModel::filterChanged,
                                                [this]() {
                                                    conversationSmartListModel_
                                                        ->fillConversationsList();
                                                    updateConversationsFilterWidget();
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
                               // current smartlist item ).
                               if (convUid != LRCInstance::getCurrentConvUid()) {
                                   return;
                               }
                               backToWelcomePage();
                           });

    searchStatusChangedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::searchStatusChanged,
                           [this](const QString& status) { emit showSearchStatus(status); });

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
}

void
ConversationsAdapter::updateConversationForNewContact(const QString& convUid)
{
    auto* convModel = LRCInstance::getCurrentConversationModel();
    if (convModel == nullptr) {
        return;
    }
    const auto selectedUid = LRCInstance::getCurrentConvUid();
    const auto conversation = convModel->getConversationForUID(convUid);
    if (!conversation.uid.isEmpty() && !conversation.participants.isEmpty()) {
        try {
            const auto contact = convModel->owner.contactModel->getContact(
                conversation.participants[0]);
            if (!contact.profileInfo.uri.isEmpty() && contact.profileInfo.uri == selectedUid) {
                LRCInstance::setSelectedConvId(convUid);
                convModel->selectConversation(convUid);
            }
        } catch (...) {
            return;
        }
    }
}
