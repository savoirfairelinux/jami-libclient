/**
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

#include "mediahandleritemlistmodel.h"

MediaHandlerItemListModel::MediaHandlerItemListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

MediaHandlerItemListModel::~MediaHandlerItemListModel() {}

int
MediaHandlerItemListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        return LRCInstance::pluginModel().listCallMediaHandlers().size();
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
MediaHandlerItemListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
MediaHandlerItemListModel::data(const QModelIndex& index, int role) const
{
    auto mediahandlerList = LRCInstance::pluginModel().listCallMediaHandlers();
    if (!index.isValid() || mediahandlerList.size() <= index.row()) {
        return QVariant();
    }

    auto details = LRCInstance::pluginModel().getCallMediaHandlerDetails(
        mediahandlerList.at(index.row()));
    auto status = LRCInstance::pluginModel().getCallMediaHandlerStatus();
    bool loaded = false;
    if (status["name"] == details.id)
        loaded = true;
    if (!details.pluginId.isEmpty()) {
        details.pluginId.remove(details.pluginId.size() - 5, 5);
    }

    switch (role) {
    case Role::MediaHandlerName:
        return QVariant(details.name);
    case Role::MediaHandlerId:
        return QVariant(mediahandlerList.at(index.row()));
    case Role::MediaHandlerIcon:
        return QVariant(details.iconPath);
    case Role::IsLoaded:
        return QVariant(loaded);
    case Role::PluginId:
        return QVariant(details.pluginId);
    }
    return QVariant();
}

QHash<int, QByteArray>
MediaHandlerItemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MediaHandlerName] = "MediaHandlerName";
    roles[MediaHandlerId] = "MediaHandlerId";
    roles[MediaHandlerIcon] = "MediaHandlerIcon";
    roles[IsLoaded] = "IsLoaded";
    roles[PluginId] = "PluginId";

    return roles;
}

QModelIndex
MediaHandlerItemListModel::index(int row, int column, const QModelIndex& parent) const
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
MediaHandlerItemListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
MediaHandlerItemListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
MediaHandlerItemListModel::reset()
{
    beginResetModel();
    endResetModel();
}
