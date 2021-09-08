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
    switch (role) {
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
        return QVariant(false);
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
        return {};
    }
    case Role::Draft: {
        if (!item.uid.isEmpty()) {
            const auto draft = lrcInstance_->getContentDraft(item.uid, item.accountId);
            if (!draft.isEmpty()) {
                // Pencil Emoji
                uint cp = 0x270F;
                auto emojiString = QString::fromUcs4(reinterpret_cast<char32_t*>(&cp), 1);
                return emojiString + draft;
            }
        }
        return {};
    }
    case Role::IsRequest:
        return QVariant(item.isRequest);
    case Role::Title:
        return QVariant(model_->title(item.uid));
    case Role::UnreadMessagesCount:
        return QVariant(item.unreadMessages);
    case Role::LastInteractionTimeStamp: {
        if (!item.interactions->empty()) {
            auto ts = static_cast<qint32>(item.interactions->at(item.lastMessageUid).timestamp);
            return QVariant(ts);
        }
        break;
    }
    case Role::LastInteractionDate: {
        if (!item.interactions->empty()) {
            return QVariant(
                Utils::formatTimeString(item.interactions->at(item.lastMessageUid).timestamp));
        }
        break;
    }
    case Role::LastInteraction: {
        if (!item.interactions->empty()) {
            return QVariant(item.interactions->at(item.lastMessageUid).body);
        }
        break;
    }
    case Role::IsSwarm:
        return QVariant(item.isSwarm());
    case Role::IsCoreDialog:
        return QVariant(item.isCoreDialog());
    case Role::Mode:
        return QVariant(static_cast<int>(item.mode));
    case Role::UID:
        return QVariant(item.uid);
    case Role::Uris:
        return QVariant(model_->peersForConversation(item.uid).toList());
    case Role::Monikers: {
        // we shouldn't ever need these individually, they are used for filtering only
        QStringList ret;
        Q_FOREACH (const auto& peerUri, model_->peersForConversation(item.uid))
            try {
                auto& accInfo = lrcInstance_->getCurrentAccountInfo();
                auto contact = accInfo.contactModel->getContact(peerUri);
                ret << contact.profileInfo.alias << contact.registeredName;
            } catch (const std::exception&) {
            }
        return ret;
    }
    case Role::ReadOnly:
        return QVariant(item.readOnly);
    default:
        break;
    }

    if (item.isCoreDialog()) {
        auto peerUriList = model_->peersForConversation(item.uid);
        if (peerUriList.isEmpty()) {
            return {};
        }
        auto peerUri = peerUriList.at(0);
        ContactModel* contactModel;
        contact::Info contact {};
        contactModel = lrcInstance_->getCurrentAccountInfo().contactModel.get();
        try {
            contact = contactModel->getContact(peerUri);
        } catch (const std::exception&) {
            qWarning() << Q_FUNC_INFO << "Can't find contact" << peerUri
                       << " this is a bug, please report";
        }

        switch (role) {
        case Role::BestId:
            return QVariant(contactModel->bestIdForContact(peerUri));
        case Role::Presence:
            return QVariant(contact.isPresent);
        case Role::Alias:
            return QVariant(contact.profileInfo.alias);
        case Role::RegisteredName:
            return QVariant(contact.registeredName);
        case Role::URI:
            return QVariant(peerUri);
        case Role::IsBanned:
            return QVariant(contact.isBanned);
        case Role::ContactType:
            return QVariant(static_cast<int>(contact.profileInfo.type));
        }
    }

    return {};
}
