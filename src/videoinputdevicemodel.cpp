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

#include "lrcinstance.h"

#include "api/newdevicemodel.h"

VideoInputDeviceModel::VideoInputDeviceModel(QObject* parent)
    : AbstractListModelBase(parent)
{}

VideoInputDeviceModel::~VideoInputDeviceModel() {}

int
VideoInputDeviceModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        return lrcInstance_->avModel().getDevices().size();
    }
    return 0;
}

QVariant
VideoInputDeviceModel::data(const QModelIndex& index, int role) const
{
    auto deviceList = lrcInstance_->avModel().getDevices();
    if (!index.isValid()) {
        return QVariant();
    }

    if (deviceList.size() <= index.row()) {
        return QVariant();
    }

    auto currentDeviceSetting = lrcInstance_->avModel().getDeviceSettings(deviceList[index.row()]);

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
    case Role::isCurrent:
        return QVariant(index.row() == getCurrentIndex());
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
    roles[isCurrent] = "isCurrent";
    return roles;
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
    return lrcInstance_->avModel().getDevices().size();
}

int
VideoInputDeviceModel::getCurrentIndex() const
{
    QString currentId = lrcInstance_->avModel().getCurrentVideoCaptureDevice();
    auto resultList = match(index(0, 0), DeviceId, QVariant(currentId));
    return resultList.size() > 0 ? resultList[0].row() : 0;
}
