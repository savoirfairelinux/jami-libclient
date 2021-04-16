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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "conversationlistmodelbase.h"
#include "selectablelistproxymodel.h"

// A wrapper view model around ConversationModel's search result data
class SearchResultsListModel : public ConversationListModelBase
{
    Q_OBJECT

public:
    explicit SearchResultsListModel(LRCInstance* instance, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void setFilter(const QString& filterString);

public Q_SLOTS:
    void onSearchResultsUpdated();
};

// The top level pre sorted and filtered model to be consumed by QML ListViews
class SearchResultsListProxyModel final : public SelectableListProxyModel
{
    Q_OBJECT

public:
    explicit SearchResultsListProxyModel(QAbstractListModel* model, QObject* parent = nullptr)
        : SelectableListProxyModel(model, parent) {};
};
