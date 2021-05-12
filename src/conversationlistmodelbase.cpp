/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "conversationlistmodelbase.h"

ConversationListModelBase::ConversationListModelBase(LRCInstance* instance, QObject* parent)
    : AbstractListModelBase(parent)
{
    lrcInstance_ = instance;
    model_ = lrcInstance_->getCurrentConversationModel();
}

int
ConversationListModelBase::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QHash<int, QByteArray>
ConversationListModelBase::roleNames() const
{
    using namespace ConversationList;
    QHash<int, QByteArray> roles;
#define X(role) roles[role] = #role;
    CONV_ROLES
#undef X
    return roles;
}

QVariant
ConversationListModelBase::dataForItem(item_t item, int role) const
{
    if (item.participants.isEmpty()) {
        return QVariant();
    }
    // WARNING: not swarm ready
    auto peerUri = item.participants[0];
    ContactModel* contactModel {nullptr};
    contact::Info contact {};
    try {
        const auto& accountInfo = lrcInstance_->getAccountInfo(item.accountId);
        contactModel = accountInfo.contactModel.get();
        contact = contactModel->getContact(peerUri);
    } catch (...) {
        return QVariant(false);
    }

    // Since we are using image provider right now, image url representation should be unique to
    // be able to use the image cache, account avatar will only be updated once PictureUid changed
    switch (role) {
    case Role::BestName:
        return QVariant(contactModel->bestNameForContact(peerUri));
    case Role::BestId:
        return QVariant(contactModel->bestIdForContact(peerUri));
    case Role::Presence:
        return QVariant(contact.isPresent);
    case Role::PictureUid:
        return QVariant(contactAvatarUidMap_[peerUri]);
    case Role::Alias:
        return QVariant(contact.profileInfo.alias);
    case Role::RegisteredName:
        return QVariant(contact.registeredName);
    case Role::URI:
        return QVariant(peerUri);
    case Role::UnreadMessagesCount:
        return QVariant(item.unreadMessages);
    case Role::LastInteractionTimeStamp: {
        if (!item.interactions.empty()) {
            auto ts = static_cast<qint32>(item.interactions.at(item.lastMessageUid).timestamp);
            return QVariant(ts);
        }
        break;
    }
    case Role::LastInteractionDate: {
        if (!item.interactions.empty()) {
            return QVariant(
                Utils::formatTimeString(item.interactions.at(item.lastMessageUid).timestamp));
        }
        break;
    }
    case Role::LastInteraction: {
        if (!item.interactions.empty()) {
            return QVariant(item.interactions.at(item.lastMessageUid).body);
        }
        break;
    }
    case Role::ContactType: {
        return QVariant(static_cast<int>(contact.profileInfo.type));
    }
    case Role::IsBanned: {
        return QVariant(contact.isBanned);
    }
    case Role::UID:
        return QVariant(item.uid);
    case Role::InCall: {
        const auto& convInfo = lrcInstance_->getConversationFromConvUid(item.uid);
        if (!convInfo.uid.isEmpty()) {
            auto* callModel = lrcInstance_->getCurrentCallModel();
            return QVariant(callModel->hasCall(convInfo.callId));
        }
        return QVariant(false);
    }
    case Role::IsAudioOnly: {
        const auto& convInfo = lrcInstance_->getConversationFromConvUid(item.uid);
        if (!convInfo.uid.isEmpty()) {
            auto* call = lrcInstance_->getCallInfoForConversation(convInfo);
            if (call) {
                return QVariant(call->isAudioOnly);
            }
        }
        return QVariant();
    }
    case Role::CallStackViewShouldShow: {
        const auto& convInfo = lrcInstance_->getConversationFromConvUid(item.uid);
        if (!convInfo.uid.isEmpty() && !convInfo.callId.isEmpty()) {
            auto* callModel = lrcInstance_->getCurrentCallModel();
            const auto& call = callModel->getCall(convInfo.callId);
            return QVariant(callModel->hasCall(convInfo.callId)
                            && ((!call.isOutgoing
                                 && (call.status == call::Status::IN_PROGRESS
                                     || call.status == call::Status::PAUSED
                                     || call.status == call::Status::INCOMING_RINGING))
                                || (call.isOutgoing && call.status != call::Status::ENDED)));
        }
        return QVariant(false);
    }
    case Role::CallState: {
        const auto& convInfo = lrcInstance_->getConversationFromConvUid(item.uid);
        if (!convInfo.uid.isEmpty()) {
            if (auto* call = lrcInstance_->getCallInfoForConversation(convInfo)) {
                return QVariant(static_cast<int>(call->status));
            }
        }
        return QVariant();
    }
    case Role::Draft: {
        if (!item.uid.isEmpty()) {
            const auto draft = lrcInstance_->getContentDraft(item.uid, item.accountId);
            if (!draft.isEmpty()) {
                // Pencil Emoji
                uint cp = 0x270F;
                auto emojiString = QString::fromUcs4(&cp, 1);
                return emojiString + draft;
            }
        }
        return QVariant("");
    }
    }
    return QVariant();
}

void
ConversationListModelBase::updateContactAvatarUid(const QString& contactUri)
{
    contactAvatarUidMap_[contactUri] = Utils::generateUid();
}

void
ConversationListModelBase::fillContactAvatarUidMap(
    const lrc::api::ContactModel::ContactInfoMap& contacts)
{
    if (contacts.size() == 0) {
        contactAvatarUidMap_.clear();
        return;
    }

    if (contactAvatarUidMap_.isEmpty() || contacts.size() != contactAvatarUidMap_.size()) {
        bool useContacts = contacts.size() > contactAvatarUidMap_.size();
        auto contactsKeyList = contacts.keys();
        auto contactAvatarUidMapKeyList = contactAvatarUidMap_.keys();

        for (int i = 0;
             i < (useContacts ? contactsKeyList.size() : contactAvatarUidMapKeyList.size());
             ++i) {
            // Insert or update
            if (i < contactsKeyList.size() && !contactAvatarUidMap_.contains(contactsKeyList.at(i)))
                contactAvatarUidMap_.insert(contactsKeyList.at(i), Utils::generateUid());
            // Remove
            if (i < contactAvatarUidMapKeyList.size()
                && !contacts.contains(contactAvatarUidMapKeyList.at(i)))
                contactAvatarUidMap_.remove(contactAvatarUidMapKeyList.at(i));
        }
    }
}
