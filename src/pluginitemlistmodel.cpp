/**
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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

#include "pluginitemlistmodel.h"

PluginItemListModel::PluginItemListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

PluginItemListModel::~PluginItemListModel() {}

int
PluginItemListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /// Count
        return LRCInstance::pluginModel().listAvailablePlugins().size();
    }
    /// A valid QModelIndex returns 0 as no entry has sub-elements.
    return 0;
}

int
PluginItemListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /// Only need one column.
    return 1;
}

QVariant
PluginItemListModel::data(const QModelIndex& index, int role) const
{
    auto pluginList = LRCInstance::pluginModel().listAvailablePlugins();
    if (!index.isValid() || pluginList.size() <= index.row()) {
        return QVariant();
    }

    auto details = LRCInstance::pluginModel().getPluginDetails(pluginList.at(index.row()));

    switch (role) {
    case Role::PluginName:
        return QVariant(details.name);
    case Role::PluginId:
        return QVariant(pluginList.at(index.row()));
    case Role::PluginIcon:
        return QVariant(details.iconPath);
    case Role::IsLoaded:
        return QVariant(details.loaded);
    }
    return QVariant();
}

QHash<int, QByteArray>
PluginItemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PluginName] = "PluginName";
    roles[PluginId] = "PluginId";
    roles[PluginIcon] = "PluginIcon";
    roles[IsLoaded] = "IsLoaded";

    return roles;
}

QModelIndex
PluginItemListModel::index(int row, int column, const QModelIndex& parent) const
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
PluginItemListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
PluginItemListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
PluginItemListModel::reset()
{
    beginResetModel();
    endResetModel();
}

int
PluginItemListModel::pluginsCount()
{
    return LRCInstance::pluginModel().listAvailablePlugins().size();
}
