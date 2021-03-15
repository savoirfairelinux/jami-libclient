/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#include "contactadapter.h"

#include "lrcinstance.h"

ContactAdapter::ContactAdapter(QObject* parent, LRCInstance* instance)
    : QmlAdapterBase(parent, instance)
{
    selectableProxyModel_.reset(new SelectableProxyModel(smartListModel_.get()));
}

QVariant
ContactAdapter::getContactSelectableModel(int type)
{
    // Called from qml every time contact picker refreshes.
    listModeltype_ = static_cast<SmartListModel::Type>(type);

    if (listModeltype_ == SmartListModel::Type::CONVERSATION) {
        defaultModerators_ = lrcInstance_->accountModel().getDefaultModerators(
            lrcInstance_->getCurrAccId());
        smartListModel_.reset(new SmartListModel(this, listModeltype_, lrcInstance_));
        smartListModel_->fillConversationsList();
    } else {
        smartListModel_.reset(new SmartListModel(this, listModeltype_, lrcInstance_));
    }
    selectableProxyModel_->setSourceModel(smartListModel_.get());

    // Adjust filter.
    switch (listModeltype_) {
    case SmartListModel::Type::CONVERSATION:
        selectableProxyModel_->setPredicate([this](const QModelIndex& index, const QRegExp&) {
            return !defaultModerators_.contains(index.data(SmartListModel::URI).toString());
        });
        break;

    case SmartListModel::Type::CONFERENCE:
        selectableProxyModel_->setPredicate([](const QModelIndex& index, const QRegExp&) {
            return index.data(SmartListModel::Presence).toBool();
        });
        break;
    case SmartListModel::Type::TRANSFER:
        selectableProxyModel_->setPredicate([this](const QModelIndex& index, const QRegExp& regexp) {
            // Exclude current sip callee and filtered contact.
            bool match = true;
            const auto& conv = lrcInstance_->getConversationFromConvUid(
                lrcInstance_->getCurrentConvUid());
            if (!conv.participants.isEmpty()) {
                QString calleeDisplayId = lrcInstance_->getAccountInfo(lrcInstance_->getCurrAccId())
                                              .contactModel->bestIdForContact(conv.participants[0]);

                QRegExp matchExcept = QRegExp(QString("\\b(?!" + calleeDisplayId + "\\b)\\w+"));
                match = matchExcept.indexIn(index.data(SmartListModel::Role::DisplayID).toString())
                        != -1;
            }

            if (match) {
                match = regexp.indexIn(index.data(SmartListModel::Role::DisplayID).toString())
                        != -1;
            }
            return match && !index.parent().isValid();
        });
        break;
    default:
        break;
    }
    selectableProxyModel_->invalidate();

    return QVariant::fromValue(selectableProxyModel_.get());
}

void
ContactAdapter::setSearchFilter(const QString& filter)
{
    if (listModeltype_ == SmartListModel::Type::CONFERENCE) {
        smartListModel_->setConferenceableFilter(filter);
    } else if (listModeltype_ == SmartListModel::Type::CONVERSATION) {
        selectableProxyModel_->setPredicate(
            [this, filter](const QModelIndex& index, const QRegExp&) {
                return (!defaultModerators_.contains(index.data(SmartListModel::URI).toString())
                        && index.data(SmartListModel::DisplayName).toString().contains(filter));
            });
    }
    selectableProxyModel_->setFilterRegExp(
        QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
}

void
ContactAdapter::contactSelected(int index)
{
    auto contactIndex = selectableProxyModel_->index(index, 0);
    auto* callModel = lrcInstance_->getCurrentCallModel();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(
        lrcInstance_->getCurrentConvUid());
    if (contactIndex.isValid()) {
        switch (listModeltype_) {
        case SmartListModel::Type::CONFERENCE: {
            // Conference.
            const auto sectionName = contactIndex.data(SmartListModel::Role::SectionName)
                                         .value<QString>();
            if (!sectionName.isEmpty()) {
                smartListModel_->toggleSection(sectionName);
                return;
            }

            const auto convUid = contactIndex.data(SmartListModel::Role::UID).value<QString>();
            const auto accId = contactIndex.data(SmartListModel::Role::AccountId).value<QString>();
            const auto callId = lrcInstance_->getCallIdForConversationUid(convUid, accId);

            if (!callId.isEmpty()) {
                if (convInfo.uid.isEmpty()) {
                    return;
                }
                auto thisCallId = convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;

                callModel->joinCalls(thisCallId, callId);
            } else {
                const auto contactUri = contactIndex.data(SmartListModel::Role::URI).value<QString>();
                auto call = lrcInstance_->getCallInfoForConversation(convInfo);
                if (!call) {
                    return;
                }
                callModel->callAndAddParticipant(contactUri, call->id, call->isAudioOnly);
            }
        } break;
        case SmartListModel::Type::TRANSFER: {
            // SIP Transfer.
            const auto contactUri = contactIndex.data(SmartListModel::Role::URI).value<QString>();

            if (convInfo.uid.isEmpty()) {
                return;
            }
            const auto callId = convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;

            QString destCallId;

            try {
                // Check if the call exist - (check non-finished calls).
                const auto callInfo = callModel->getCallFromURI(contactUri, true);
                destCallId = callInfo.id;
            } catch (std::exception& e) {
                qDebug().noquote() << e.what();
                destCallId = "";
            }

            // If no second call -> blind transfer.
            // If there is a second call -> attended transfer.
            if (destCallId.size() == 0) {
                callModel->transfer(callId, "sip:" + contactUri);
                callModel->hangUp(callId);
            } else {
                callModel->transferToCall(callId, destCallId);
                callModel->hangUp(callId);
                callModel->hangUp(destCallId);
            }
        } break;
        case SmartListModel::Type::CONVERSATION: {
            const auto contactUri = contactIndex.data(SmartListModel::Role::URI).value<QString>();
            if (contactUri.isEmpty()) {
                return;
            }

            lrcInstance_->accountModel().setDefaultModerator(lrcInstance_->getCurrAccId(),
                                                             contactUri,
                                                             true);
            emit defaultModeratorsUpdated();

        } break;
        default:
            break;
        }
    }
}
