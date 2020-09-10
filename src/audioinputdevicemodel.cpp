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

#include "audioinputdevicemodel.h"

AudioInputDeviceModel::AudioInputDeviceModel(QObject* parent)
    : QAbstractListModel(parent)
{}

AudioInputDeviceModel::~AudioInputDeviceModel() {}

int
AudioInputDeviceModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        return LRCInstance::avModel().getAudioInputDevices().size();
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
AudioInputDeviceModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
AudioInputDeviceModel::data(const QModelIndex& index, int role) const
{
    auto deviceList = LRCInstance::avModel().getAudioInputDevices();
    if (!index.isValid() || deviceList.size() <= index.row()) {
        return QVariant();
    }

    switch (role) {
    case Role::Device_ID:
        return QVariant(deviceList.at(index.row()));
    case Role::ID_UTF8:
        return QVariant(deviceList.at(index.row()).toUtf8());
    }
    return QVariant();
}

QHash<int, QByteArray>
AudioInputDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Device_ID] = "Device_ID";
    roles[ID_UTF8] = "ID_UTF8";
    return roles;
}

QModelIndex
AudioInputDeviceModel::index(int row, int column, const QModelIndex& parent) const
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
AudioInputDeviceModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
AudioInputDeviceModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
AudioInputDeviceModel::reset()
{
    beginResetModel();
    endResetModel();
}

int
AudioInputDeviceModel::getCurrentSettingIndex()
{
    QString currentId = LRCInstance::avModel().getInputDevice();
    auto resultList = match(index(0, 0), Device_ID, QVariant(currentId));

    int resultRowIndex = 0;
    if (resultList.size() > 0) {
        resultRowIndex = resultList[0].row();
    }

    return resultRowIndex;
}
