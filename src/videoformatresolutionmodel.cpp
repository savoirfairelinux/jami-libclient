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

#include "videoformatresolutionmodel.h"

VideoFormatResolutionModel::VideoFormatResolutionModel(QObject* parent)
    : QAbstractListModel(parent)
{}

VideoFormatResolutionModel::~VideoFormatResolutionModel() {}

int
VideoFormatResolutionModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        QString currentDeviceId = LRCInstance::avModel().getCurrentVideoCaptureDevice();
        auto deviceCapabilities = LRCInstance::avModel().getDeviceCapabilities(currentDeviceId);
        if (deviceCapabilities.size() == 0) {
            return 0;
        }
        try {
            auto currentSettings = LRCInstance::avModel().getDeviceSettings(currentDeviceId);
            auto currentChannel = currentSettings.channel;
            currentChannel = currentChannel.isEmpty() ? "default" : currentChannel;
            auto channelCaps = deviceCapabilities[currentChannel];

            return channelCaps.size();
        } catch (const std::exception& e) {
            qWarning() << e.what();
            return 0;
        }
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
VideoFormatResolutionModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
VideoFormatResolutionModel::data(const QModelIndex& index, int role) const
{
    try {
        QString currentDeviceId = LRCInstance::avModel().getCurrentVideoCaptureDevice();
        auto deviceCapabilities = LRCInstance::avModel().getDeviceCapabilities(currentDeviceId);

        auto currentSettings = LRCInstance::avModel().getDeviceSettings(currentDeviceId);
        auto currentChannel = currentSettings.channel;
        currentChannel = currentChannel.isEmpty() ? "default" : currentChannel;
        auto channelCaps = deviceCapabilities[currentChannel];

        if (!index.isValid() || channelCaps.size() <= index.row()
            || deviceCapabilities.size() == 0) {
            return QVariant();
        }

        switch (role) {
        case Role::Resolution:
            return QVariant(channelCaps.at(index.row()).first);
        case Role::Resolution_UTF8:
            return QVariant(channelCaps.at(index.row()).first.toUtf8());
        }

    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    return QVariant();
}

QHash<int, QByteArray>
VideoFormatResolutionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Resolution] = "Resolution";
    roles[Resolution_UTF8] = "Resolution_UTF8";
    return roles;
}

QModelIndex
VideoFormatResolutionModel::index(int row, int column, const QModelIndex& parent) const
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
VideoFormatResolutionModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
VideoFormatResolutionModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
VideoFormatResolutionModel::reset()
{
    beginResetModel();
    endResetModel();
}

int
VideoFormatResolutionModel::getCurrentSettingIndex()
{
    int resultRowIndex = 0;
    try {
        QString currentDeviceId = LRCInstance::avModel().getCurrentVideoCaptureDevice();
        auto currentSettings = LRCInstance::avModel().getDeviceSettings(currentDeviceId);
        QString currentResolution = currentSettings.size;
        auto resultList = match(index(0, 0), Resolution, QVariant(currentResolution));

        if (resultList.size() > 0) {
            resultRowIndex = resultList[0].row();
        }

    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    return resultRowIndex;
}
