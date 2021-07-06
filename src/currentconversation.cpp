/*
 * Copyright (C) 2021 by Savoir-faire Linux
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "currentconversation.h"

CurrentConversation::CurrentConversation(LRCInstance* lrcInstance, QObject* parent)
    : QObject(parent)
    , lrcInstance_(lrcInstance)
{
    // whenever the account changes, reconnect the new conversation model
    // for updates to the conversation and call state/id
    connect(lrcInstance_,
            &LRCInstance::currentAccountIdChanged,
            this,
            &CurrentConversation::connectModel);
    connectModel();

    // update when the conversation itself changes
    connect(lrcInstance_,
            &LRCInstance::selectedConvUidChanged,
            this,
            &CurrentConversation::updateData);
    updateData();
}

void
CurrentConversation::updateData()
{
    auto convId = lrcInstance_->get_selectedConvUid();
    if (convId.isEmpty())
        return;
    set_id(convId);
    try {
        auto accountId = lrcInstance_->get_currentAccountId();
        const auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId);
        if (auto optConv = accInfo.conversationModel->getConversationForUid(convId)) {
            auto& convInfo = optConv->get();
            set_title(accInfo.conversationModel->title(convId));
            set_uris(accInfo.conversationModel->peersForConversation(convId).toList());
            set_isSwarm(convInfo.isSwarm());
            set_isLegacy(convInfo.isLegacy());
            set_isCoreDialog(convInfo.isCoreDialog());
            set_isRequest(convInfo.isRequest);
            set_readOnly(convInfo.readOnly);
            set_needsSyncing(convInfo.needsSyncing);
            set_isSip(accInfo.profileInfo.type == profile::Type::SIP);
            set_callId(convInfo.getCallId());
            set_allMessagesLoaded(convInfo.allMessagesLoaded);
            if (accInfo.callModel->hasCall(callId_)) {
                auto call = accInfo.callModel->getCall(callId_);
                set_callState(call.status);
            } else {
                set_callState(call::Status::INVALID);
            }
            set_inCall(callState_ == call::Status::CONNECTED
                       || callState_ == call::Status::IN_PROGRESS
                       || callState_ == call::Status::PAUSED);

            // The temporary status is only for dialogs.
            // It can be used to display add contact/conversation UI and
            // is consistently determined by the peer's uri being equal to
            // the conversation id.
            set_isTemporary(isCoreDialog_ ? convId == uris_.at(0) : false);

            auto isContact {false};
            if (isCoreDialog_)
                try {
                    auto& contact = accInfo.contactModel->getContact(uris_.at(0));
                    isContact = contact.profileInfo.type != profile::Type::TEMPORARY;
                } catch (...) {
                    qInfo() << "Contact not found";
                }
            set_isContact(isContact);
        }
    } catch (...) {
        qWarning() << "Can't update current conversation data for" << convId;
    }
}

void
CurrentConversation::onConversationUpdated(const QString& convId)
{
    // filter for our currently set id
    if (id_ != convId)
        return;
    updateData();
}

void
CurrentConversation::connectModel()
{
    auto convModel = lrcInstance_->getCurrentConversationModel();
    if (!convModel)
        return;

    connect(lrcInstance_->getCurrentConversationModel(),
            &ConversationModel::conversationUpdated,
            this,
            &CurrentConversation::onConversationUpdated,
            Qt::UniqueConnection);
}
