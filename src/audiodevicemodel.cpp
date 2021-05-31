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

#include "audiodevicemodel.h"

#include "lrcinstance.h"

AudioDeviceModel::AudioDeviceModel(QObject* parent)
    : AbstractListModelBase(parent)
{
    connect(this, &AudioDeviceModel::typeChanged, this, &AudioDeviceModel::reset);
}

int
AudioDeviceModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return devices_.size();
    }
    return 0;
}

int
AudioDeviceModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant
AudioDeviceModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || devices_.size() <= index.row()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
    case Role::DeviceName: {
        auto deviceName = devices_.at(index.row());
        QRegularExpression re("{{(.*?)}}");
        QRegularExpressionMatch match = re.match(deviceName);
        if (match.hasMatch() && re.captureCount() > 0) {
            deviceName.replace(match.captured(0), QObject::tr(match.captured(1).toUtf8()));
        }
        return QVariant(deviceName.toUtf8());
    }
    case Role::RawDeviceName:
        return QVariant(devices_.at(index.row()));
    case Role::isCurrent:
        return QVariant(index.row() == getCurrentIndex());
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray>
AudioDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DeviceName] = "DeviceName";
    roles[RawDeviceName] = "RawDeviceName";
    return roles;
}

QModelIndex
AudioDeviceModel::index(int row, int column, const QModelIndex&) const
{
    if (column != 0) {
        return QModelIndex();
    }
    if (row >= 0 && row < rowCount()) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex
AudioDeviceModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

Qt::ItemFlags
AudioDeviceModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
AudioDeviceModel::reset()
{
    beginResetModel();
    devices_ = type_ == Type::Record ? lrcInstance_->avModel().getAudioInputDevices()
                                     : lrcInstance_->avModel().getAudioOutputDevices();
    endResetModel();
}

int
AudioDeviceModel::getCurrentIndex() const
{
    auto currentId = type_ == Type::Record ? lrcInstance_->avModel().getInputDevice()
                                           : lrcInstance_->avModel().getOutputDevice();
    auto resultList = match(index(0, 0), Qt::DisplayRole, QVariant(currentId));
    return resultList.size() > 0 ? resultList[0].row() : 0;
}
