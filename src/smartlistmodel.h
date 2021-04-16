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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "conversationlistmodelbase.h"

namespace ContactList {
Q_NAMESPACE
enum Type { CONVERSATION, CONFERENCE, TRANSFER, COUNT__ };
Q_ENUM_NS(Type)
} // namespace ContactList

using namespace lrc::api;
class LRCInstance;

class SmartListModel : public ConversationListModelBase
{
    Q_OBJECT
public:
    using Type = ContactList::Type;

    explicit SmartListModel(QObject* parent = nullptr,
                            Type listModelType = Type::CONVERSATION,
                            LRCInstance* instance = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row,
                      int column = 0,
                      const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Q_INVOKABLE void setConferenceableFilter(const QString& filter = {});
    Q_INVOKABLE void toggleSection(const QString& section);
    Q_INVOKABLE int currentUidSmartListModelIndex();
    Q_INVOKABLE void fillConversationsList();

private:
    Type listModelType_;
    QMap<QString, bool> sectionState_;
    QMap<ConferenceableItem, ConferenceableValue> conferenceables_;
    ConversationModel::ConversationQueueProxy conversations_;
};
