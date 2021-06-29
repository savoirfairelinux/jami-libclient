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

#include <QSortFilterProxyModel>

// The base class for a filtered and sorted model.
// The model may be part of a group and if so, will track a
// mutually exclusive selection.
class SelectableListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_PROPERTY(int, currentFilteredRow)

public:
    explicit SelectableListProxyModel(QAbstractListModel* model, QObject* parent = nullptr);

    void bindSourceModel(QAbstractListModel* model);

    Q_INVOKABLE void setFilter(const QString& filterString);
    Q_INVOKABLE void select(const QModelIndex& index);
    Q_INVOKABLE void select(int row);
    Q_INVOKABLE void deselect();
    Q_INVOKABLE QVariant dataForRow(int row, int role) const;
    void selectSourceRow(int row);

    // this may not be the best place for this but it prevents a level of
    // inheritance and prevents code duplication
    Q_INVOKABLE void updateContactAvatarUid(const QString& contactUri);

public Q_SLOTS:
    void updateSelection(bool rowsRemoved = false);

Q_SIGNALS:
    void validSelectionChanged();

private Q_SLOTS:
    void onModelUpdated();
    void onModelTrimmed();

private:
    QPersistentModelIndex selectedSourceIndex_;
};

class SelectableListProxyGroupModel : public QObject
{
    Q_OBJECT
public:
    explicit SelectableListProxyGroupModel(QList<SelectableListProxyModel*> models,
                                           QObject* parent = nullptr);
    QList<SelectableListProxyModel*> models_;
};
