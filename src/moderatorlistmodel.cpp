/*
 * Copyright (C) 2019-2022 Savoir-faire Linux Inc.
 * Author: Albert Babí Oller <albert.babi@savoirfairelinux.com>
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

#include "moderatorlistmodel.h"

#include "lrcinstance.h"

ModeratorListModel::ModeratorListModel(QObject* parent)
    : AbstractListModelBase(parent)
{}

ModeratorListModel::~ModeratorListModel() {}

int
ModeratorListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        return lrcInstance_->accountModel()
            .getDefaultModerators(lrcInstance_->getCurrentAccountInfo().id)
            .size();
    }
    return 0;
}

int
ModeratorListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
ModeratorListModel::data(const QModelIndex& index, int role) const
{
    try {
        QStringList list = lrcInstance_->accountModel().getDefaultModerators(
                    lrcInstance_->get_currentAccountId());
        if (!index.isValid() || list.size() <= index.row()) {
            return QVariant();
        }
        auto contactInfo = lrcInstance_->getCurrentAccountInfo().contactModel->getContact(
            list.at(index.row()));

        switch (role) {
        case Role::ContactName: {
            QString str = lrcInstance_->getCurrentAccountInfo().contactModel->
                    bestNameForContact(list.at(index.row()));
            return QVariant(str);
        }
        case Role::ContactID:
            return QVariant(contactInfo.profileInfo.uri);
        }
    } catch (const std::exception& e) {
        qDebug() << e.what();
    }
    return QVariant();
}

QHash<int, QByteArray>
ModeratorListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ContactName] = "ContactName";
    roles[ContactID] = "ContactID";
    return roles;
}

QModelIndex
ModeratorListModel::index(int row, int column, const QModelIndex& parent) const
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
ModeratorListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
ModeratorListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
ModeratorListModel::reset()
{
    beginResetModel();
    endResetModel();
}
