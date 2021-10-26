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

#include "videodevices.h"

// VideoInputDeviceModel
VideoInputDeviceModel::VideoInputDeviceModel(LRCInstance* lrcInstance,
                                             VideoDevices* videoDeviceInstance)
    : QAbstractListModel(videoDeviceInstance)
    , lrcInstance_(lrcInstance)
    , videoDevices_(videoDeviceInstance)
{}

VideoInputDeviceModel::~VideoInputDeviceModel() {}

int
VideoInputDeviceModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        return videoDevices_->get_listSize();
    }
    return 0;
}

QVariant
VideoInputDeviceModel::data(const QModelIndex& index, int role) const
{
    auto deviceList = lrcInstance_->avModel().getDevices();
    if (!index.isValid() || deviceList.size() == 0 || index.row() >= deviceList.size()) {
        return QVariant();
    }

    auto currentDeviceSetting = lrcInstance_->avModel().getDeviceSettings(deviceList[index.row()]);

    switch (role) {
    case Role::DeviceName:
        return QVariant(currentDeviceSetting.name);
    case Role::DeviceId:
        return QVariant(currentDeviceSetting.id);
    }
    return QVariant();
}

QHash<int, QByteArray>
VideoInputDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DeviceName] = "DeviceName";
    roles[DeviceId] = "DeviceId";
    return roles;
}

int
VideoInputDeviceModel::getCurrentIndex() const
{
    QString currentId = videoDevices_->get_defaultId();
    auto resultList = match(index(0, 0), DeviceId, QVariant(currentId));
    return resultList.size() > 0 ? resultList[0].row() : 0;
}

// VideoFormatResolutionModel
VideoFormatResolutionModel::VideoFormatResolutionModel(LRCInstance* lrcInstance,
                                                       VideoDevices* videoDeviceInstance)
    : QAbstractListModel(videoDeviceInstance)
    , lrcInstance_(lrcInstance)
    , videoDevices_(videoDeviceInstance)
{}

VideoFormatResolutionModel::~VideoFormatResolutionModel() {}

int
VideoFormatResolutionModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        return videoDevices_->get_defaultResRateList().size();
    }
    return 0;
}

QVariant
VideoFormatResolutionModel::data(const QModelIndex& index, int role) const
{
    auto& channelCaps = videoDevices_->get_defaultResRateList();
    if (!index.isValid() || channelCaps.size() <= index.row() || channelCaps.size() == 0) {
        return QVariant();
    }

    switch (role) {
    case Role::Resolution:
        return QVariant(channelCaps.at(index.row()).first);
    }

    return QVariant();
}

QHash<int, QByteArray>
VideoFormatResolutionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Resolution] = "Resolution";
    return roles;
}

int
VideoFormatResolutionModel::getCurrentIndex() const
{
    QString currentDeviceId = videoDevices_->get_defaultId();
    QString currentResolution = videoDevices_->get_defaultRes();
    auto resultList = match(index(0, 0), Resolution, QVariant(currentResolution));

    return resultList.size() > 0 ? resultList[0].row() : 0;
}

// VideoFormatFpsModel
VideoFormatFpsModel::VideoFormatFpsModel(LRCInstance* lrcInstance, VideoDevices* videoDeviceInstance)
    : QAbstractListModel(videoDeviceInstance)
    , lrcInstance_(lrcInstance)
    , videoDevices_(videoDeviceInstance)
{}

VideoFormatFpsModel::~VideoFormatFpsModel() {}

int
VideoFormatFpsModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        return videoDevices_->get_defaultFpsList().size();
    }
    return 0;
}

QVariant
VideoFormatFpsModel::data(const QModelIndex& index, int role) const
{
    auto& fpsList = videoDevices_->get_defaultFpsList();
    if (!index.isValid() || fpsList.size() == 0 || index.row() >= fpsList.size()) {
        return QVariant();
    }

    switch (role) {
    case Role::FPS:
        return QVariant(static_cast<int>(fpsList[index.row()]));
    case Role::FPS_Float:
        return QVariant(fpsList[index.row()]);
    }

    return QVariant();
}

QHash<int, QByteArray>
VideoFormatFpsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FPS] = "FPS";
    roles[FPS_Float] = "FPS_Float";
    return roles;
}

int
VideoFormatFpsModel::getCurrentIndex() const
{
    QString currentDeviceId = videoDevices_->get_defaultId();
    float currentFps = videoDevices_->get_defaultFps();
    auto resultList = match(index(0, 0), FPS, QVariant(currentFps));

    return resultList.size() > 0 ? resultList[0].row() : 0;
}

// VideoDevices
VideoDevices::VideoDevices(LRCInstance* lrcInstance, QObject* parent)
    : QObject(parent)
    , lrcInstance_(lrcInstance)
    , devicesFilterModel_(new CurrentItemFilterModel(this))
    , resFilterModel_(new CurrentItemFilterModel(this))
    , fpsFilterModel_(new CurrentItemFilterModel(this))
{
    devicesSourceModel_ = new VideoInputDeviceModel(lrcInstance, this);
    resSourceModel_ = new VideoFormatResolutionModel(lrcInstance, this);
    fpsSourceModel_ = new VideoFormatFpsModel(lrcInstance, this);

    devicesFilterModel_->setSourceModel(devicesSourceModel_);
    resFilterModel_->setSourceModel(resSourceModel_);
    fpsFilterModel_->setSourceModel(fpsSourceModel_);

    devicesFilterModel_->setFilterRole(VideoInputDeviceModel::DeviceName);
    resFilterModel_->setFilterRole(VideoFormatResolutionModel::Resolution);
    fpsFilterModel_->setFilterRole(VideoFormatFpsModel::FPS);

    connect(&lrcInstance_->avModel(),
            &lrc::api::AVModel::deviceEvent,
            this,
            &VideoDevices::onVideoDeviceEvent);

    auto displaySettings = lrcInstance_->avModel().getDeviceSettings(DEVICE_DESKTOP);

    auto desktopfpsSource = lrcInstance_->avModel().getDeviceCapabilities(DEVICE_DESKTOP);
    if (desktopfpsSource.contains(CHANNEL_DEFAULT) && !desktopfpsSource[CHANNEL_DEFAULT].empty()) {
        desktopfpsSourceModel_ = desktopfpsSource[CHANNEL_DEFAULT][0].second;
        if (desktopfpsSourceModel_.indexOf(displaySettings.rate) >= 0)
            set_screenSharingDefaultFps(displaySettings.rate);
    }
    updateData();
}

VideoDevices::~VideoDevices() {}

QVariant
VideoDevices::devicesFilterModel()
{
    return QVariant::fromValue(devicesFilterModel_);
}

QVariant
VideoDevices::devicesSourceModel()
{
    return QVariant::fromValue(devicesSourceModel_);
}

QVariant
VideoDevices::resFilterModel()
{
    return QVariant::fromValue(resFilterModel_);
}

QVariant
VideoDevices::resSourceModel()
{
    return QVariant::fromValue(resSourceModel_);
}

QVariant
VideoDevices::fpsFilterModel()
{
    return QVariant::fromValue(fpsFilterModel_);
}

QVariant
VideoDevices::fpsSourceModel()
{
    return QVariant::fromValue(fpsSourceModel_);
}

void
VideoDevices::setDefaultDevice(int index, bool useSourceModel)
{
    QString deviceId {};
    auto callId = lrcInstance_->getCurrentCallId();

    if (useSourceModel)
        deviceId = devicesSourceModel_
                       ->data(devicesSourceModel_->index(index, 0), VideoInputDeviceModel::DeviceId)
                       .toString();
    else
        deviceId = devicesFilterModel_
                       ->data(devicesFilterModel_->index(index, 0), VideoInputDeviceModel::DeviceId)
                       .toString();

    lrcInstance_->avModel().setDefaultDevice(deviceId);

    if (!callId.isEmpty())
        lrcInstance_->getCurrentCallModel()->switchInputTo(deviceId, callId);

    updateData();
}

const QString
VideoDevices::getDefaultDevice()
{
    auto idx = devicesSourceModel_->getCurrentIndex();
    auto rendererId = QString("camera://")
                      + devicesSourceModel_
                            ->data(devicesSourceModel_->index(idx, 0),
                                   VideoInputDeviceModel::DeviceId)
                            .toString();
    return rendererId;
}

#pragma optimize("", off)
QString
VideoDevices::startDevice(const QString& deviceId, bool force)
{
    if (deviceId.isEmpty())
        return {};
    lrcInstance_->renderer()->addDistantRenderer(deviceId);
    deviceOpen_ = true;
    return lrcInstance_->renderer()->startPreviewing(deviceId, force);
}

void
VideoDevices::stopDevice(const QString& deviceId, bool force)
{
    if (!deviceId.isEmpty() && (!lrcInstance_->hasActiveCall(true) || force)) {
        lrcInstance_->renderer()->stopPreviewing(deviceId);
        lrcInstance_->renderer()->removeDistantRenderer(deviceId);
        deviceOpen_ = false;
    }
}
#pragma optimize("", on)

void
VideoDevices::setDefaultDeviceRes(int index)
{
    auto& channelCaps = get_defaultResRateList();
    auto settings = lrcInstance_->avModel().getDeviceSettings(get_defaultId());
    settings.size = resFilterModel_
                        ->data(resFilterModel_->index(index, 0),
                               VideoFormatResolutionModel::Resolution)
                        .toString();

    for (int i = 0; i < channelCaps.size(); i++) {
        if (channelCaps[i].first == settings.size) {
            settings.rate = channelCaps[i].second.at(0);
            lrcInstance_->avModel().setDeviceSettings(settings);
            break;
        }
    }

    updateData();
}

void
VideoDevices::setDefaultDeviceFps(int index)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(get_defaultId());
    settings.size = get_defaultRes();
    settings.rate = fpsFilterModel_
                        ->data(fpsFilterModel_->index(index, 0), VideoFormatFpsModel::FPS_Float)
                        .toFloat();

    lrcInstance_->avModel().setDeviceSettings(settings);

    updateData();
}

void
VideoDevices::setDisplayFPS(const QString& fps)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(DEVICE_DESKTOP);
    settings.id = DEVICE_DESKTOP;
    settings.rate = fps.toInt();
    lrcInstance_->avModel().setDeviceSettings(settings);
    set_screenSharingDefaultFps(fps.toInt());
}

QVariant
VideoDevices::getScreenSharingFpsModel()
{
    return QVariant::fromValue(desktopfpsSourceModel_.toList());
}

void
VideoDevices::updateData()
{
    set_listSize(lrcInstance_->avModel().getDevices().size());

    if (get_listSize() != 0) {
        auto defaultDevice = lrcInstance_->avModel().getDefaultDevice();
        auto defaultDeviceSettings = lrcInstance_->avModel().getDeviceSettings(defaultDevice);
        auto defaultDeviceCap = lrcInstance_->avModel().getDeviceCapabilities(defaultDevice);
        auto currentResRateList = defaultDeviceCap[defaultDeviceSettings.channel.isEmpty()
                                                       ? CHANNEL_DEFAULT
                                                       : defaultDeviceSettings.channel];
        lrc::api::video::FrameratesList fpsList;

        for (int i = 0; i < currentResRateList.size(); i++) {
            if (currentResRateList[i].first == defaultDeviceSettings.size) {
                fpsList = currentResRateList[i].second;
            }
        }

        if (deviceOpen_ && defaultId_ != defaultDeviceSettings.id) {
            auto callId = lrcInstance_->getCurrentCallId();
            if (!callId.isEmpty()) {
                auto callId = lrcInstance_->getCurrentCallId();
                auto callInfos = lrcInstance_->getCallInfo(callId,
                                                           lrcInstance_->get_currentAccountId());
                for (const auto& media : callInfos->mediaList) {
                    if (media["MUTED"] == "false" && media["ENABLED"] == "true"
                        && media["SOURCE"] == getDefaultDevice()) {
                        /*lrcInstance_->avModel().switchInputTo("camera://" +
                           defaultDeviceSettings.id, callId);*/
                        // startDevice("camera://" + defaultDeviceSettings.id);
                        break;
                    }
                }
            }
        }

        set_defaultChannel(defaultDeviceSettings.channel);
        set_defaultId(defaultDeviceSettings.id);
        set_defaultName(defaultDeviceSettings.name);
        set_defaultRes(defaultDeviceSettings.size);
        set_defaultFps(defaultDeviceSettings.rate);
        set_defaultResRateList(currentResRateList);
        set_defaultFpsList(fpsList);

        devicesFilterModel_->setCurrentItemFilter(defaultDeviceSettings.name);
        resFilterModel_->setCurrentItemFilter(defaultDeviceSettings.size);
        fpsFilterModel_->setCurrentItemFilter(static_cast<int>(defaultDeviceSettings.rate));
    } else {
        set_defaultChannel("");
        set_defaultId("");
        set_defaultName("");
        set_defaultRes("");
        set_defaultFps(0);
        set_defaultResRateList({});
        set_defaultFpsList({});

        devicesFilterModel_->setCurrentItemFilter("");
        resFilterModel_->setCurrentItemFilter("");
        fpsFilterModel_->setCurrentItemFilter(0);
    }

    devicesSourceModel_->reset();
    resSourceModel_->reset();
    fpsSourceModel_->reset();
}

void
VideoDevices::onVideoDeviceEvent()
{
    auto& avModel = lrcInstance_->avModel();
    auto* callModel = lrcInstance_->getCurrentCallModel();
    auto defaultDevice = avModel.getDefaultDevice();
    QString callId = lrcInstance_->getCurrentCallId();

    // Decide whether a device has plugged, unplugged, or nothing has changed.
    auto deviceList = avModel.getDevices();
    auto currentDeviceListSize = deviceList.size();
    auto previousDeviceListSize = get_listSize();

    DeviceEvent deviceEvent {DeviceEvent::None};
    if (currentDeviceListSize > previousDeviceListSize) {
        if (previousDeviceListSize == 0)
            deviceEvent = DeviceEvent::FirstDevice;
        else
            deviceEvent = DeviceEvent::Added;
    } else if (currentDeviceListSize < previousDeviceListSize) {
        deviceEvent = DeviceEvent::Removed;
    }

    auto cb = [this, currentDeviceListSize, deviceEvent, defaultDevice, callId] {
        auto& avModel = lrcInstance_->avModel();
        auto* callModel = lrcInstance_->getCurrentCallModel();
        if (currentDeviceListSize == 0) {
            callModel->switchInputTo({}, callId);
            avModel.stopPreview(this->getDefaultDevice());
        } else if (deviceEvent == DeviceEvent::Removed) {
            callModel->switchInputTo(defaultDevice, callId);
        }

        updateData();
        Q_EMIT deviceListChanged(currentDeviceListSize);
    };

    if (deviceEvent == DeviceEvent::Added) {
        updateData();
        Q_EMIT deviceListChanged(currentDeviceListSize);
    } else if (deviceEvent == DeviceEvent::FirstDevice) {
        updateData();

        if (callId.isEmpty()) {
            Q_EMIT deviceAvailable();
        } else {
            callModel->switchInputTo(defaultDevice, callId);
        }

        Q_EMIT deviceListChanged(currentDeviceListSize);
    } else if (deviceOpen_) {
        updateData();

        // Use QueuedConnection to make sure that it happens at the event loop of current device
        Utils::oneShotConnect(
            lrcInstance_->renderer(),
            &RenderManager::distantRenderingStopped,
            this,
            [this, cb](const QString& id) {
                if (this->getDefaultDevice() == id)
                    cb();
            },
            Qt::QueuedConnection);
    } else {
        cb();
    }
}

const lrc::api::video::ResRateList&
VideoDevices::get_defaultResRateList()
{
    return defaultResRateList_;
}

void
VideoDevices::set_defaultResRateList(lrc::api::video::ResRateList resRateList)
{
    defaultResRateList_.swap(resRateList);
}

const lrc::api::video::FrameratesList&
VideoDevices::get_defaultFpsList()
{
    return defaultFpsList_;
}

void
VideoDevices::set_defaultFpsList(lrc::api::video::FrameratesList rateList)
{
    defaultFpsList_.swap(rateList);
}
