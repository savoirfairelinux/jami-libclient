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

#include "videoformatfpsmodel.h"

#include "lrcinstance.h"

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/newdevicemodel.h"

VideoFormatFpsModel::VideoFormatFpsModel(QObject* parent)
    : AbstractListModelBase(parent)
{
    connect(this, &AbstractListModelBase::lrcInstanceChanged, [this] {
        if (lrcInstance_)
            try {
                QString currentDeviceId = lrcInstance_->avModel().getCurrentVideoCaptureDevice();
                auto currentSettings = lrcInstance_->avModel().getDeviceSettings(currentDeviceId);
                currentResolution_ = currentSettings.size;
            } catch (const std::exception& e) {
                qWarning() << "Constructor of VideoFormatFpsModel, exception: " << e.what();
            }
    });
}

VideoFormatFpsModel::~VideoFormatFpsModel() {}

int
VideoFormatFpsModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        /*
         * Count.
         */
        QString currentDeviceId = lrcInstance_->avModel().getCurrentVideoCaptureDevice();
        auto deviceCapabilities = lrcInstance_->avModel().getDeviceCapabilities(currentDeviceId);
        if (deviceCapabilities.size() == 0) {
            return 0;
        }
        try {
            auto currentSettings = lrcInstance_->avModel().getDeviceSettings(currentDeviceId);
            auto currentChannel = currentSettings.channel;
            currentChannel = currentChannel.isEmpty() ? "default" : currentChannel;
            auto channelCaps = deviceCapabilities[currentChannel];

            bool resolutionFound = false;
            int indexOfCurrentResolutionInResRateList = 0;

            for (int i = 0; i < channelCaps.size(); i++) {
                if (channelCaps[i].first == currentResolution_) {
                    indexOfCurrentResolutionInResRateList = i;
                    resolutionFound = true;
                    break;
                }
            }

            if (resolutionFound) {
                auto fpsList = channelCaps[indexOfCurrentResolutionInResRateList].second;
                return fpsList.size();
            }
        } catch (const std::exception& e) {
            qWarning() << e.what();
        }
        return 0;
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
VideoFormatFpsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
VideoFormatFpsModel::data(const QModelIndex& index, int role) const
{
    try {
        QString currentDeviceId = lrcInstance_->avModel().getCurrentVideoCaptureDevice();
        auto deviceCapabilities = lrcInstance_->avModel().getDeviceCapabilities(currentDeviceId);

        auto currentSettings = lrcInstance_->avModel().getDeviceSettings(currentDeviceId);
        auto currentChannel = currentSettings.channel;
        currentChannel = currentChannel.isEmpty() ? "default" : currentChannel;
        auto channelCaps = deviceCapabilities[currentChannel];

        bool resolutionFound = false;
        int indexOfCurrentResolutionInResRateList = 0;

        for (int i = 0; i < channelCaps.size(); i++) {
            if (channelCaps[i].first == currentResolution_) {
                indexOfCurrentResolutionInResRateList = i;
                resolutionFound = true;
                break;
            }
        }

        if (!index.isValid() || channelCaps.size() <= index.row() || deviceCapabilities.size() == 0
            || !resolutionFound) {
            return QVariant();
        }

        auto fpsList = channelCaps[indexOfCurrentResolutionInResRateList].second;

        switch (role) {
        case Role::FPS:
            return QVariant(fpsList[index.row()]);
        case Role::FPS_ToDisplay_UTF8:
            QString rateDisplayUtf8 = QString("%1 fps").arg((int) fpsList[index.row()]);
            return QVariant(rateDisplayUtf8.toUtf8());
        }

    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    return QVariant();
}

QHash<int, QByteArray>
VideoFormatFpsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FPS] = "FPS";
    roles[FPS_ToDisplay_UTF8] = "FPS_ToDisplay_UTF8";
    return roles;
}

QModelIndex
VideoFormatFpsModel::index(int row, int column, const QModelIndex& parent) const
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
VideoFormatFpsModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
VideoFormatFpsModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
VideoFormatFpsModel::reset()
{
    beginResetModel();
    endResetModel();
}

int
VideoFormatFpsModel::getCurrentSettingIndex()
{
    int resultRowIndex = 0;
    try {
        QString currentDeviceId = lrcInstance_->avModel().getCurrentVideoCaptureDevice();
        auto currentSettings = lrcInstance_->avModel().getDeviceSettings(currentDeviceId);
        float currentFps = currentSettings.rate;
        auto resultList = match(index(0, 0), FPS, QVariant(currentFps));

        if (resultList.size() > 0) {
            resultRowIndex = resultList[0].row();
        }

    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    return resultRowIndex;
}

QString
VideoFormatFpsModel::getCurrentResolution()
{
    return currentResolution_;
}

void
VideoFormatFpsModel::setCurrentResolution(QString resNew)
{
    if (currentResolution_ != resNew) {
        currentResolution_ = resNew;
        reset();
        emit currentResolutionChanged(resNew);
    }
}
