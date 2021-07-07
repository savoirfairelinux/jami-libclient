/*
 * Copyright (C) 2021 by Savoir-faire Linux
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

#pragma once

#include "conversationlistmodelbase.h"
#include "selectablelistproxymodel.h"

#include "api/profile.h"

#include <QSortFilterProxyModel>

// A wrapper view model around ConversationModel's underlying data
class ConversationListModel final : public ConversationListModelBase
{
    Q_OBJECT

public:
    explicit ConversationListModel(LRCInstance* instance, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

// The top level filtered and sorted model to be consumed by QML ListViews
class ConversationListProxyModel final : public SelectableListProxyModel
{
    Q_OBJECT

public:
    explicit ConversationListProxyModel(QAbstractListModel* model, QObject* parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    Q_INVOKABLE void setFilterRequests(bool filterRequests);

private:
    // This flag can be toggled when switching tabs to show the current account's
    // conversation invites.
    bool filterRequests_ {false};
};
