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
            set_title(accInfo.conversationModel->title(convId));
            set_uris(accInfo.conversationModel->peersForConversation(convId).toList());
            set_isSwarm(optConv->get().isSwarm());
            set_isLegacy(optConv->get().isLegacy());
            set_isCoreDialog(optConv->get().isCoreDialog());
            set_isRequest(optConv->get().isRequest);
            set_readOnly(optConv->get().readOnly);
            set_needsSyncing(optConv->get().needsSyncing);
            set_isSip(accInfo.profileInfo.type == profile::Type::SIP);
            set_callId(optConv->get().getCallId());
            if (accInfo.callModel->hasCall(callId_)) {
                set_callState(accInfo.callModel->getCall(callId_).status);
            } else {
                set_callState(call::Status::INVALID);
            }
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
