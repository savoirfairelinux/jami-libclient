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

#include "searchresultslistmodel.h"

SearchResultsListModel::SearchResultsListModel(LRCInstance* instance, QObject* parent)
    : ConversationListModelBase(instance, parent)
{}

int
SearchResultsListModel::rowCount(const QModelIndex& parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (!parent.isValid() && model_) {
        return model_->getAllSearchResults().size();
    }
    return 0;
}

QVariant
SearchResultsListModel::data(const QModelIndex& index, int role) const
{
    const auto& data = model_->getAllSearchResults();
    if (!index.isValid() || data.empty())
        return {};
    return dataForItem(data.at(index.row()), role);
}

void
SearchResultsListModel::setFilter(const QString& filterString)
{
    model_->setFilter(filterString);
}

void
SearchResultsListModel::onSearchResultsUpdated()
{
    beginResetModel();
    endResetModel();
}
