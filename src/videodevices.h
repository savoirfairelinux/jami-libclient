/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

#pragma once

#include "lrcinstance.h"
#include "qtutils.h"

#include "api/newdevicemodel.h"

#include <QSortFilterProxyModel>
#include <QObject>

class VideoDevices;

class CurrentItemFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CurrentItemFilterModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)

    {}

    void setCurrentItemFilter(const QVariant& filter)
    {
        currentItemFilter_ = filter;
    }

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        // Do not filter if there is only one item.
        if (currentItemFilter_.isNull() || sourceModel()->rowCount() == 1)
            return true;

        // Exclude current item filter.
        auto index = sourceModel()->index(sourceRow, 0, sourceParent);
        return index.data(filterRole()) != currentItemFilter_ && !index.parent().isValid();
    }

private:
    QVariant currentItemFilter_ {};
};

class VideoInputDeviceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role { DeviceName = Qt::UserRole + 1, DeviceId };
    Q_ENUM(Role)

    explicit VideoInputDeviceModel(LRCInstance* lrcInstance, VideoDevices* videoDeviceInstance);
    ~VideoInputDeviceModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reset()
    {
        beginResetModel();
        endResetModel();
    }

    // Get model index of the current device
    Q_INVOKABLE int getCurrentIndex() const;

private:
    LRCInstance* lrcInstance_ {nullptr};
    VideoDevices* const videoDevices_;
};

class VideoFormatResolutionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role { Resolution = Qt::UserRole + 1 };
    Q_ENUM(Role)

    explicit VideoFormatResolutionModel(LRCInstance* lrcInstance, VideoDevices* videoDeviceInstance);
    ~VideoFormatResolutionModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reset()
    {
        beginResetModel();
        endResetModel();
    }

    // Get model index of the current device
    Q_INVOKABLE int getCurrentIndex() const;

private:
    LRCInstance* lrcInstance_ {nullptr};
    VideoDevices* const videoDevices_;
};

class VideoFormatFpsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role { FPS = Qt::UserRole + 1, FPS_Float };
    Q_ENUM(Role)

    explicit VideoFormatFpsModel(LRCInstance* lrcInstance, VideoDevices* videoDeviceInstance);
    ~VideoFormatFpsModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reset()
    {
        beginResetModel();
        endResetModel();
    }

    // Get model index of the current device
    Q_INVOKABLE int getCurrentIndex() const;

private:
    LRCInstance* lrcInstance_;
    VideoDevices* const videoDevices_;
};

class VideoDevices : public QObject
{
    Q_OBJECT
    QML_RO_PROPERTY(int, listSize)

    QML_RO_PROPERTY(QString, defaultChannel)
    QML_RO_PROPERTY(QString, defaultId)
    QML_RO_PROPERTY(QString, defaultName)
    QML_RO_PROPERTY(QString, defaultRes)
    QML_RO_PROPERTY(int, defaultFps)

public:
    explicit VideoDevices(LRCInstance* lrcInstance, QObject* parent = nullptr);
    ~VideoDevices();

    Q_INVOKABLE QVariant devicesFilterModel();
    Q_INVOKABLE QVariant devicesSourceModel();

    Q_INVOKABLE QVariant resFilterModel();
    Q_INVOKABLE QVariant resSourceModel();

    Q_INVOKABLE QVariant fpsFilterModel();
    Q_INVOKABLE QVariant fpsSourceModel();

    Q_INVOKABLE void setDefaultDevice(int index, bool useSourceModel = false);
    Q_INVOKABLE void setDefaultDeviceRes(int index);
    Q_INVOKABLE void setDefaultDeviceFps(int index);

    const lrc::api::video::ResRateList& get_defaultResRateList();
    void set_defaultResRateList(lrc::api::video::ResRateList resRateList);

    const lrc::api::video::FrameratesList& get_defaultFpsList();
    void set_defaultFpsList(lrc::api::video::FrameratesList rateList);

Q_SIGNALS:
    void deviceAvailable();
    void deviceListChanged(int inputs);

private Q_SLOTS:
    void onVideoDeviceEvent();

private:
    // Used to classify capture device events.
    enum class DeviceEvent { FirstDevice, Added, Removed, None };

    void updateData();

    LRCInstance* lrcInstance_;

    CurrentItemFilterModel* devicesFilterModel_;
    CurrentItemFilterModel* resFilterModel_;
    CurrentItemFilterModel* fpsFilterModel_;

    VideoInputDeviceModel* devicesSourceModel_;
    VideoFormatResolutionModel* resSourceModel_;
    VideoFormatFpsModel* fpsSourceModel_;

    lrc::api::video::ResRateList defaultResRateList_;
    lrc::api::video::FrameratesList defaultFpsList_;
};
