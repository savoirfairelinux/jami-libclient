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

#include "videoinputdevicemodel.h"

VideoInputDeviceModel::VideoInputDeviceModel(QObject* parent)
    : QAbstractListModel(parent)
{}

VideoInputDeviceModel::~VideoInputDeviceModel() {}

int
VideoInputDeviceModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        int deviceListSize = LRCInstance::avModel().getDevices().size();
        if (deviceListSize > 0) {
            return deviceListSize;
        } else {
            return 1;
        }
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
VideoInputDeviceModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
VideoInputDeviceModel::data(const QModelIndex& index, int role) const
{
    auto deviceList = LRCInstance::avModel().getDevices();
    if (!index.isValid()) {
        return QVariant();
    }

    if (deviceList.size() == 0 && index.row() == 0) {
        switch (role) {
        case Role::DeviceName:
            return QVariant(QObject::tr("No device"));
        case Role::DeviceName_UTF8:
            return QVariant(QObject::tr("No device").toUtf8());
        }
    }

    if (deviceList.size() <= index.row()) {
        return QVariant();
    }

    auto currentDeviceSetting = LRCInstance::avModel().getDeviceSettings(deviceList[index.row()]);

    switch (role) {
    case Role::DeviceChannel:
        return QVariant((QString) currentDeviceSetting.channel);
    case Role::DeviceName:
        return QVariant(currentDeviceSetting.name);
    case Role::DeviceId:
        return QVariant(currentDeviceSetting.id);
    case Role::CurrentFrameRate:
        return QVariant((float) currentDeviceSetting.rate);
    case Role::CurrentResolution:
        return QVariant((QString) currentDeviceSetting.size);
    case Role::DeviceName_UTF8:
        return QVariant(currentDeviceSetting.name.toUtf8());
    }
    return QVariant();
}

QHash<int, QByteArray>
VideoInputDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DeviceChannel] = "DeviceChannel";
    roles[DeviceName] = "DeviceName";
    roles[DeviceId] = "DeviceId";
    roles[CurrentFrameRate] = "CurrentFrameRate";
    roles[CurrentResolution] = "CurrentResolution";
    roles[DeviceName_UTF8] = "DeviceName_UTF8";
    return roles;
}

QModelIndex
VideoInputDeviceModel::index(int row, int column, const QModelIndex& parent) const
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
VideoInputDeviceModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
VideoInputDeviceModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
VideoInputDeviceModel::reset()
{
    beginResetModel();
    endResetModel();
}

int
VideoInputDeviceModel::deviceCount()
{
    return LRCInstance::avModel().getDevices().size();
}

int
VideoInputDeviceModel::getCurrentSettingIndex()
{
    QString currentId = LRCInstance::avModel().getCurrentVideoCaptureDevice();
    auto resultList = match(index(0, 0), DeviceId, QVariant(currentId));

    int resultRowIndex = 0;
    if (resultList.size() > 0) {
        resultRowIndex = resultList[0].row();
    }

    return resultRowIndex;
}
