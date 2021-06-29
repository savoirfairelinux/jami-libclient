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

#include "selectablelistproxymodel.h"

SelectableListProxyModel::SelectableListProxyModel(QAbstractListModel* model, QObject* parent)
    : QSortFilterProxyModel(parent)
    , currentFilteredRow_(-1)
    , selectedSourceIndex_(QModelIndex())
{
    bindSourceModel(model);
}

void
SelectableListProxyModel::bindSourceModel(QAbstractListModel* model)
{
    setSourceModel(model);
    connect(sourceModel(),
            &QAbstractListModel::dataChanged,
            this,
            &SelectableListProxyModel::onModelUpdated,
            Qt::UniqueConnection);
    connect(model,
            &QAbstractListModel::rowsInserted,
            this,
            &SelectableListProxyModel::onModelUpdated,
            Qt::UniqueConnection);
    connect(model,
            &QAbstractListModel::rowsRemoved,
            this,
            &SelectableListProxyModel::onModelTrimmed,
            Qt::UniqueConnection);
    connect(sourceModel(),
            &QAbstractListModel::modelReset,
            this,
            &SelectableListProxyModel::deselect,
            Qt::UniqueConnection);
}

void
SelectableListProxyModel::setFilter(const QString& filterString)
{
    setFilterFixedString(filterString);
    updateSelection();
}

void
SelectableListProxyModel::select(const QModelIndex& index)
{
    selectedSourceIndex_ = mapToSource(index);
    updateSelection();
}

void
SelectableListProxyModel::select(int row)
{
    select(index(row, 0));
}

void
SelectableListProxyModel::deselect()
{
    selectedSourceIndex_ = QModelIndex();
    currentFilteredRow_ = -1;
    Q_EMIT currentFilteredRowChanged();
}

QVariant
SelectableListProxyModel::dataForRow(int row, int role) const
{
    return data(index(row, 0), role);
}

void
SelectableListProxyModel::selectSourceRow(int row)
{
    // note: the convId <-> index binding loop present
    // is broken here
    if (row == -1 || selectedSourceIndex_.row() == row)
        return;
    selectedSourceIndex_ = sourceModel()->index(row, 0);
    updateSelection();
}

void
SelectableListProxyModel::updateSelection(bool rowsRemoved)
{
    // if there has been no valid selection made, there is
    // nothing to update
    if (!selectedSourceIndex_.isValid() && currentFilteredRow_ == -1)
        return;

    auto lastFilteredRow = currentFilteredRow_;
    auto filteredIndex = mapFromSource(selectedSourceIndex_);

    // if the source model is empty, invalidate the selection
    if (rowCount() == 0 && rowsRemoved) {
        set_currentFilteredRow(-1);
        Q_EMIT validSelectionChanged();
        return;
    }

    // if the source and filtered index is no longer valid
    // this would indicate that a mutation has occured,
    // thus any arbritrary ux decision is okay here
    if (!selectedSourceIndex_.isValid()) {
        auto row = qMax(--currentFilteredRow_, 0);
        selectedSourceIndex_ = mapToSource(index(row, 0));
        filteredIndex = mapFromSource(selectedSourceIndex_);
        currentFilteredRow_ = filteredIndex.row();
        Q_EMIT currentFilteredRowChanged();
        Q_EMIT validSelectionChanged();
        return;
    }

    // update the row for ListView observers
    set_currentFilteredRow(filteredIndex.row());

    // finally, if the filter index is invalid, then we have
    // probably just filtered out the selected item and don't
    // want to force reselection of other ui components, as the
    // source index is still valid, in that case, or if the
    // row hasn't changed, don't notify
    if (filteredIndex.isValid() && lastFilteredRow != currentFilteredRow_) {
        Q_EMIT validSelectionChanged();
    }
}

void
SelectableListProxyModel::onModelUpdated()
{
    updateSelection();
}

void
SelectableListProxyModel::onModelTrimmed()
{
    updateSelection(true);
}

SelectableListProxyGroupModel::SelectableListProxyGroupModel(QList<SelectableListProxyModel*> models,
                                                             QObject* parent)
    : QObject(parent)
    , models_(models)
{
    Q_FOREACH (auto* m, models_) {
        connect(m, &SelectableListProxyModel::validSelectionChanged, [this, m] {
            // deselct all other lists in the group
            Q_FOREACH (auto* otherM, models_) {
                if (m != otherM) {
                    otherM->deselect();
                }
            }
        });
    }
}
