/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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

#include "preferenceitemlistmodel.h"
#include <map>

std::map<QString, int> mapType {{QString("List"), PreferenceItemListModel::Type::LIST}};

PreferenceItemListModel::PreferenceItemListModel(QObject *parent)
    : QAbstractListModel(parent)
{}

PreferenceItemListModel::~PreferenceItemListModel() {}

int
PreferenceItemListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        return LRCInstance::pluginModel().getPluginPreferences(pluginId_).size();
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
PreferenceItemListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
PreferenceItemListModel::data(const QModelIndex &index, int role) const
{
    auto preferenceList = LRCInstance::pluginModel().getPluginPreferences(pluginId_);
    if (!index.isValid() || preferenceList.size() <= index.row()) {
        return QVariant();
    }    

    auto details = preferenceList.at(index.row());
    int type = Type::DEFAULT;
    auto it = mapType.find(details["type"]);
    if (it != mapType.end())
    {
        type = mapType[details["type"]];
    }

    switch (role) {
        case Role::PreferenceKey:
            return QVariant(details["key"]);
        case Role::PreferenceName:
            return QVariant(details["title"]);
        case Role::PreferenceSummary:
            return QVariant(details["summary"]);
        case Role::PreferenceType:
            return QVariant(type);
        case Role::PreferenceDefaultValue:
            return QVariant(details["defaultValue"]);
        case Role::PreferenceEntries:
            return QVariant(details["entries"]);
        case Role::PreferenceEntryValues:
            return QVariant(details["entryValues"]);
    }
    return QVariant();
}

QHash<int, QByteArray>
PreferenceItemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PreferenceKey] = "PreferenceKey";
    roles[PreferenceName] = "PreferenceName";
    roles[PreferenceSummary] = "PreferenceSummary";
    roles[PreferenceType] = "PreferenceType";
    roles[PreferenceDefaultValue] = "PreferenceDefaultValue";
    roles[PreferenceEntries] = "PreferenceEntries";
    roles[PreferenceEntryValues] = "PreferenceEntryValues";
    
    return roles;
}

QModelIndex
PreferenceItemListModel::index(int row, int column, const QModelIndex &parent) const
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
PreferenceItemListModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
PreferenceItemListModel::flags(const QModelIndex &index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
PreferenceItemListModel::reset()
{
    beginResetModel();
    endResetModel();
}

QString
PreferenceItemListModel::pluginId() const
{
    return pluginId_;
}

void
PreferenceItemListModel::setPluginId(const QString &pluginId)
{
    pluginId_ = pluginId;
}
