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
#include "utils.h"

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/conversationmodel.h"
#include "api/contactmodel.h"

#include <QDateTime>

SmartListModel::SmartListModel(QObject* parent,
                               SmartListModel::Type listModelType,
                               LRCInstance* instance)
    : ConversationListModelBase(instance, parent)
    , listModelType_(listModelType)
{
    if (listModelType_ == Type::CONFERENCE) {
        setConferenceableFilter();
    }
}

int
SmartListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        auto& accInfo = lrcInstance_->accountModel().getAccountInfo(
            lrcInstance_->getCurrentAccountId());
        auto& convModel = accInfo.conversationModel;
        if (listModelType_ == Type::TRANSFER) {
            return convModel->getFilteredConversations(accInfo.profileInfo.type).size();
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
        return conversations_.size();
    }
    return 0;
}

QVariant
SmartListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    switch (listModelType_) {
    case Type::TRANSFER: {
        try {
            auto& currentAccountInfo = lrcInstance_->accountModel().getAccountInfo(
                lrcInstance_->getCurrentAccountId());
            auto& convModel = currentAccountInfo.conversationModel;

            auto& item = convModel->getFilteredConversations(currentAccountInfo.profileInfo.type)
                             .at(index.row());
            return dataForItem(item, role);
        } catch (const std::exception& e) {
            qWarning() << e.what();
        }
    } break;
    case Type::CONFERENCE: {
        auto calls = conferenceables_[ConferenceableItem::CALL];
        auto contacts = conferenceables_[ConferenceableItem::CONTACT];
        QString itemConvUid {}, itemAccountId {};
        if (calls.size() == 0) {
            itemConvUid = contacts.at(index.row()).at(0).convId;
            itemAccountId = contacts.at(index.row()).at(0).accountId;
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
                    itemAccountId = calls.at(idx).at(0).accountId;
                }
            } else if (index.row() < contactSectionEnd) {
                if (index.row() == callSectionEnd) {
                    return QVariant(role == Role::SectionName
                                        ? (contactsOpen ? "➖ " : "➕ ") + QString(tr("Contacts"))
                                        : "");
                } else {
                    auto idx = index.row() - (callSectionEnd + 1);
                    itemConvUid = contacts.at(idx).at(0).convId;
                    itemAccountId = contacts.at(idx).at(0).accountId;
                }
            }
        }
        if (role == Role::AccountId) {
            return QVariant(itemAccountId);
        }

        auto& item = lrcInstance_->getConversationFromConvUid(itemConvUid, itemAccountId);
        return dataForItem(item, role);
    } break;
    case Type::CONVERSATION: {
        auto& item = conversations_.at(index.row());
        return dataForItem(item, role);
    } break;
    default:
        break;
    }
    return {};
}

void
SmartListModel::setConferenceableFilter(const QString& filter)
{
    beginResetModel();
    auto& accountInfo = lrcInstance_->accountModel().getAccountInfo(
        lrcInstance_->getCurrentAccountId());
    auto& convModel = accountInfo.conversationModel;
    conferenceables_ = convModel->getConferenceableConversations(lrcInstance_->get_selectedConvUid(),
                                                                 filter);
    sectionState_[tr("Calls")] = true;
    sectionState_[tr("Contacts")] = true;
    endResetModel();
}

void
SmartListModel::fillConversationsList()
{
    beginResetModel();
    fillContactAvatarUidMap(lrcInstance_->getCurrentAccountInfo().contactModel->getAllContacts());

    auto* convModel = lrcInstance_->getCurrentConversationModel();
    using ConversationList = ConversationModel::ConversationQueueProxy;
    conversations_ = ConversationList(convModel->getAllSearchResults())
                     + convModel->allFilteredConversations();
    endResetModel();
}

void
SmartListModel::toggleSection(const QString& section)
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
    const auto convUid = lrcInstance_->get_selectedConvUid();
    for (int i = 0; i < rowCount(); i++) {
        if (convUid == data(index(i, 0), Role::UID))
            return i;
    }

    return -1;
}

QModelIndex
SmartListModel::index(int row, int column, const QModelIndex& parent) const
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

Qt::ItemFlags
SmartListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    auto type = static_cast<lrc::api::profile::Type>(data(index, Role::ContactType).value<int>());
    auto uid = data(index, Role::UID).value<QString>();
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    } else if ((type == lrc::api::profile::Type::TEMPORARY && uid.isEmpty())) {
        flags &= ~(Qt::ItemIsSelectable);
    }
    return flags;
}
