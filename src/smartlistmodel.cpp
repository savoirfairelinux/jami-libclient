/*
 * Copyright (C) 2017-2020 by Savoir-faire Linux
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
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

#include "smartlistmodel.h"

#include "lrcinstance.h"
#include "pixbufmanipulator.h"
#include "utils.h"

#include "api/contactmodel.h"
#include "globalinstances.h"

#include <QDateTime>

SmartListModel::SmartListModel(const QString &accId,
                               QObject *parent,
                               SmartListModel::Type listModelType,
                               const QString &convUid)
    : QAbstractListModel(parent)
    , accountId_(accId)
    , listModelType_(listModelType)
    , convUid_(convUid)
{
    if (listModelType_ == Type::CONFERENCE) {
        setConferenceableFilter();
    }
}

SmartListModel::~SmartListModel() {}

int
SmartListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        auto &accInfo = LRCInstance::accountModel().getAccountInfo(accountId_);
        auto &convModel = accInfo.conversationModel;
        if (listModelType_ == Type::TRANSFER) {
            auto filterType = accInfo.profileInfo.type;
            return convModel->getFilteredConversations(filterType).size();
        } else if (listModelType_ == Type::CONFERENCE) {
            auto calls = conferenceables_[ConferenceableItem::CALL];
            auto contacts = conferenceables_[ConferenceableItem::CONTACT];
            auto rowCount = contacts.size();
            if (calls.size()) {
                rowCount = 2;
                rowCount += sectionState_[tr("Calls")] ? calls.size() : 0;
                rowCount += sectionState_[tr("Contacts")] ? contacts.size() : 0;
            }
            return rowCount;
        }
        return accInfo.conversationModel->allFilteredConversations().size()
                + accInfo.conversationModel->getAllSearchResults().size();
    }
    return 0;
}

int
SmartListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant
SmartListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    try {
        auto &accountInfo = LRCInstance::accountModel().getAccountInfo(accountId_);
        auto &convModel = accountInfo.conversationModel;
        lrc::api::conversation::Info item;
        if (listModelType_ == Type::TRANSFER) {
            auto filterType = accountInfo.profileInfo.type;
            item = convModel->getFilteredConversations(filterType).at(index.row());
            return getConversationItemData(item, accountInfo, role);
        } else if (listModelType_ == Type::CONFERENCE) {
            auto calls = conferenceables_[ConferenceableItem::CALL];
            auto contacts = conferenceables_[ConferenceableItem::CONTACT];
            QString itemConvUid{}, itemAccId{};
            if (calls.size() == 0) {
                itemConvUid = contacts.at(index.row()).at(0).convId;
                itemAccId = contacts.at(index.row()).at(0).accountId;
            } else {
                bool callsOpen = sectionState_[tr("Calls")];
                bool contactsOpen = sectionState_[tr("Contacts")];
                auto callSectionEnd = callsOpen ? calls.size() + 1 : 1;
                auto contactSectionEnd = contactsOpen ? callSectionEnd + contacts.size() + 1
                                                      : callSectionEnd + 1;
                if (index.row() < callSectionEnd) {
                    if (index.row() == 0) {
                        return QVariant(role == Role::SectionName
                                            ? (callsOpen ? "➖ " : "➕ ") + QString(tr("Calls"))
                                            : "");
                    } else {
                        auto idx = index.row() - 1;
                        itemConvUid = calls.at(idx).at(0).convId;
                        itemAccId = calls.at(idx).at(0).accountId;
                    }
                } else if (index.row() < contactSectionEnd) {
                    if (index.row() == callSectionEnd) {
                        return QVariant(role == Role::SectionName
                                            ? (contactsOpen ? "➖ " : "➕ ") + QString(tr("Contacts"))
                                            : "");
                    } else {
                        auto idx = index.row() - (callSectionEnd + 1);
                        itemConvUid = contacts.at(idx).at(0).convId;
                        itemAccId = contacts.at(idx).at(0).accountId;
                    }
                }
            }
            if (role == Role::AccountId) {
                return QVariant(itemAccId);
            }
            auto &itemAccountInfo = LRCInstance::accountModel().getAccountInfo(itemAccId);
            item = itemAccountInfo.conversationModel->getConversationForUID(itemConvUid);
            return getConversationItemData(item, itemAccountInfo, role);
        } else if (listModelType_ == Type::CONVERSATION) {
            item = conversations_.at(index.row());
            return getConversationItemData(item, accountInfo, role);
        }
    } catch (const std::exception &e) {
        qWarning() << e.what();
    }
    return QVariant();
}

QHash<int, QByteArray>
SmartListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayName] = "DisplayName";
    roles[DisplayID] = "DisplayID";
    roles[Picture] = "Picture";
    roles[Presence] = "Presence";
    roles[URI] = "URI";
    roles[UnreadMessagesCount] = "UnreadMessagesCount";
    roles[LastInteractionDate] = "LastInteractionDate";
    roles[LastInteraction] = "LastInteraction";
    roles[ContactType] = "ContactType";
    roles[UID] = "UID";
    roles[InCall] = "InCall";
    roles[IsAudioOnly] = "IsAudioOnly";
    roles[CallStackViewShouldShow] = "CallStackViewShouldShow";
    roles[CallStateStr] = "CallStateStr";
    roles[SectionName] = "SectionName";
    roles[AccountId] = "AccountId";
    roles[Draft] = "Draft";
    return roles;
}

void
SmartListModel::setConferenceableFilter(const QString &filter)
{
    beginResetModel();
    auto &accountInfo = LRCInstance::accountModel().getAccountInfo(accountId_);
    auto &convModel = accountInfo.conversationModel;
    conferenceables_ = convModel->getConferenceableConversations(convUid_, filter);
    sectionState_[tr("Calls")] = true;
    sectionState_[tr("Contacts")] = true;
    endResetModel();
}

void
SmartListModel::fillConversationsList()
{
    beginResetModel();
    auto* convModel = LRCInstance::getCurrentConversationModel();
    conversations_.clear();

    for (auto convSearch : convModel->getAllSearchResults()) {
        conversations_.emplace_back(convSearch);
    }

    for (auto convFilt : convModel->allFilteredConversations()) {
        conversations_.emplace_back(convFilt);
    }
    endResetModel();
}

void
SmartListModel::updateConversation(const QString &convUid)
{
    auto* convModel = LRCInstance::getCurrentConversationModel();
    for (lrc::api::conversation::Info &conversation : conversations_) {
        if (conversation.uid == convUid) {
            conversation = convModel->getConversationForUID(convUid);
            return;
        }
    }
}

void
SmartListModel::toggleSection(const QString &section)
{
    beginResetModel();
    if (section.contains(tr("Calls"))) {
        sectionState_[tr("Calls")] ^= true;
    } else if (section.contains(tr("Contacts"))) {
        sectionState_[tr("Contacts")] ^= true;
    }
    endResetModel();
}

int
SmartListModel::currentUidSmartListModelIndex()
{
    const auto convUid = LRCInstance::getCurrentConvUid();
    for (int i = 0; i < rowCount(); i++) {
        if (convUid == data(index(i, 0), Role::UID))
            return i;
    }

    return -1;
}

QVariant
SmartListModel::getConversationItemData(const conversation::Info &item,
                                        const account::Info &accountInfo,
                                        int role) const
{
    if (item.participants.size() <= 0) {
        return QVariant();
    }
    auto &contactModel = accountInfo.contactModel;
    switch (role) {
    case Role::Picture: {
        auto contactImage
            = GlobalInstances::pixmapManipulator().decorationRole(item, accountInfo).value<QImage>();
        return QString::fromLatin1(Utils::QImageToByteArray(contactImage).toBase64().data());
    }
    case Role::DisplayName: {
        if (!item.participants.isEmpty()) {
            auto &contact = contactModel->getContact(item.participants[0]);
            return QVariant(Utils::bestNameForContact(contact));
        }
        return QVariant("");
    }
    case Role::DisplayID: {
        if (!item.participants.isEmpty()) {
            auto &contact = contactModel->getContact(item.participants[0]);
            return QVariant(Utils::bestIdForContact(contact));
        }
        return QVariant("");
    }
    case Role::Presence: {
        if (!item.participants.isEmpty()) {
            auto &contact = contactModel->getContact(item.participants[0]);
            return QVariant(contact.isPresent);
        }
        return QVariant(false);
    }
    case Role::URI: {
        if (!item.participants.isEmpty()) {
            auto &contact = contactModel->getContact(item.participants[0]);
            return QVariant(contact.profileInfo.uri);
        }
        return QVariant("");
    }
    case Role::UnreadMessagesCount:
        return QVariant(item.unreadMessages);
    case Role::LastInteractionDate: {
        if (!item.interactions.empty()) {
            auto &date = item.interactions.at(item.lastMessageUid).timestamp;
            return QVariant(QString::fromStdString(Utils::formatTimeString(date)));
        }
        return QVariant("");
    }
    case Role::LastInteraction: {
        if (!item.interactions.empty()) {
            return QVariant(item.interactions.at(item.lastMessageUid).body);
        }
        return QVariant("");
    }
    case Role::LastInteractionType: {
        if (!item.interactions.empty()) {
            return QVariant(
                Utils::toUnderlyingValue(item.interactions.at(item.lastMessageUid).type));
        }
        return QVariant(0);
    }
    case Role::ContactType: {
        if (!item.participants.isEmpty()) {
            auto &contact = contactModel->getContact(item.participants[0]);
            return QVariant(Utils::toUnderlyingValue(contact.profileInfo.type));
        }
        return QVariant(0);
    }
    case Role::UID:
        return QVariant(item.uid);
    case Role::InCall: {
        auto* convModel = LRCInstance::getCurrentConversationModel();
        const auto convInfo = convModel->getConversationForUID(item.uid);
        if (!convInfo.uid.isEmpty()) {
            auto* callModel = LRCInstance::getCurrentCallModel();
            return QVariant(callModel->hasCall(convInfo.callId));
        }
        return QVariant(false);
    }
    case Role::IsAudioOnly: {
        auto* convModel = LRCInstance::getCurrentConversationModel();
        const auto convInfo = convModel->getConversationForUID(item.uid);
        if (!convInfo.uid.isEmpty()) {
            auto* call = LRCInstance::getCallInfoForConversation(convInfo);
            if (call) {
                return QVariant(call->isAudioOnly);
            }
        }
        return QVariant();
    }
    case Role::CallStackViewShouldShow: {
        auto* convModel = LRCInstance::getCurrentConversationModel();
        const auto convInfo = convModel->getConversationForUID(item.uid);
        if (!convInfo.uid.isEmpty()) {
            auto* callModel = LRCInstance::getCurrentCallModel();
            const auto call = callModel->getCall(convInfo.callId);
            return QVariant(callModel->hasCall(convInfo.callId)
                            && ((!call.isOutgoing
                                 && (call.status == lrc::api::call::Status::IN_PROGRESS
                                     || call.status == lrc::api::call::Status::PAUSED))
                                || call.isOutgoing));
        }
        return QVariant(false);
    }
    case Role::CallStateStr: {
        auto* convModel = LRCInstance::getCurrentConversationModel();
        const auto convInfo = convModel->getConversationForUID(item.uid);
        if (!convInfo.uid.isEmpty()) {
            auto* call = LRCInstance::getCallInfoForConversation(convInfo);
            if (call) {
                auto statusString = call::to_string(call->status);
                return QVariant(statusString);
            }
        }
        return QVariant();
    }
    case Role::SectionName:
        return QVariant(QString());
    case Role::Draft: {
        if (!item.uid.isEmpty()) {
            const auto draft = LRCInstance::getContentDraft(item.uid, accountInfo.id);
            if (!draft.isEmpty()) {
                /*
                 * Pencil Emoji
                 */
                uint cp = 0x270F;
                auto emojiString = QString::fromUcs4(&cp, 1);
                return emojiString + LRCInstance::getContentDraft(item.uid, accountInfo.id);
            }
        }
        return QVariant("");
    }
    }
    return QVariant();
}

QModelIndex
SmartListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (column != 0) {
        return QModelIndex();
    }

    if (row >= 0 && row < rowCount()) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex
SmartListModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
SmartListModel::flags(const QModelIndex &index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    auto type = Utils::toEnum<lrc::api::profile::Type>(data(index, Role::ContactType).value<int>());
    auto uid = data(index, Role::UID).value<QString>();
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    } else if ((type == lrc::api::profile::Type::TEMPORARY && uid.isEmpty())) {
        flags &= ~(Qt::ItemIsSelectable);
    }
    return flags;
}

void
SmartListModel::setAccount(const QString &accountId)
{
    beginResetModel();
    accountId_ = accountId;
    endResetModel();
}
