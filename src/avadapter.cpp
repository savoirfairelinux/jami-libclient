/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author : Edric Ladent Milaret<edric.ladent - milaret @savoirfairelinux.com>
 * Author : Andreas Traczyk<andreas.traczyk @savoirfairelinux.com>
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

#include "avadapter.h"

#include "lrcinstance.h"
#include "qtutils.h"

#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QScreen>

AvAdapter::AvAdapter(QObject* parent)
    : QmlAdapterBase(parent)
{
    auto& avModel = LRCInstance::avModel();

    deviceListSize_ = avModel.getDevices().size();
    connect(&avModel, &lrc::api::AVModel::deviceEvent, this, &AvAdapter::slotDeviceEvent);
}

QVariantMap
AvAdapter::populateVideoDeviceContextMenuItem()
{
    auto activeDevice = LRCInstance::avModel().getCurrentVideoCaptureDevice();

    /*
     * Create a list of video input devices.
     */
    QVariantMap deciveContextMenuNeededInfo;
    auto devices = LRCInstance::avModel().getDevices();
    for (int i = 0; i < devices.size(); i++) {
        try {
            auto settings = LRCInstance::avModel().getDeviceSettings(devices[i]);
            deciveContextMenuNeededInfo[settings.name] = QVariant(settings.id == activeDevice);
        } catch (...) {
            qDebug().noquote() << "Error in getting device settings";
        }
    }

    /*
     * Add size parameter into the map since in qml there is no way to get the size.
     */
    deciveContextMenuNeededInfo["size"] = QVariant(deciveContextMenuNeededInfo.size());

    return deciveContextMenuNeededInfo;
}

void
AvAdapter::onVideoContextMenuDeviceItemClicked(const QString& deviceName)
{
    auto* convModel = LRCInstance::getCurrentConversationModel();
    const auto conversation = convModel->getConversationForUID(LRCInstance::getCurrentConvUid());
    auto call = LRCInstance::getCallInfoForConversation(conversation);
    if (!call)
        return;

    auto deviceId = LRCInstance::avModel().getDeviceIdFromName(deviceName);
    if (deviceId.isEmpty()) {
        qWarning() << "Couldn't find device: " << deviceName;
        return;
    }
    LRCInstance::avModel().setCurrentVideoCaptureDevice(deviceId);
    LRCInstance::avModel().switchInputTo(deviceId, call->id);
}

void
AvAdapter::shareEntireScreen(int screenNumber)
{
    QScreen* screen = qApp->screens().at(screenNumber);
    if (!screen)
        return;
    QRect rect = screen ? screen->geometry() : qApp->primaryScreen()->geometry();
    LRCInstance::avModel().setDisplay(screenNumber, rect.x(), rect.y(), rect.width(), rect.height());
}

const QString
AvAdapter::captureScreen(int screenNumber)
{
    QScreen* screen = qApp->screens().at(screenNumber);
    if (!screen)
        return QString("");
    /*
     * The screen window id is always 0.
     */
    auto pixmap = screen->grabWindow(0);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return Utils::byteArrayToBase64String(buffer.data());
}

void
AvAdapter::shareFile(const QString& filePath)
{
    LRCInstance::avModel().setInputFile(filePath);
}

void
AvAdapter::shareScreenArea(int screenNumber, int x, int y, int width, int height)
{
    QScreen* screen = qApp->screens().at(screenNumber);
    if (!screen)
        return;
    QRect rect = screen ? screen->geometry() : qApp->primaryScreen()->geometry();

    /*
     * Provide minimum width, height.
     * Need to add screen x, y initial value to the setDisplay api call.
     */
    LRCInstance::avModel().setDisplay(screenNumber,
                                      rect.x() + x,
                                      rect.y() + y,
                                      width < 128 ? 128 : width,
                                      height < 128 ? 128 : height);
}

void
AvAdapter::startAudioMeter(bool async)
{
    LRCInstance::startAudioMeter(async);
}

void
AvAdapter::stopAudioMeter(bool async)
{
    LRCInstance::stopAudioMeter(async);
}

void
AvAdapter::slotDeviceEvent()
{
    auto& avModel = LRCInstance::avModel();
    auto defaultDevice = avModel.getDefaultDevice();
    auto currentCaptureDevice = avModel.getCurrentVideoCaptureDevice();
    QString callId {};

    auto* convModel = LRCInstance::getCurrentConversationModel();
    const auto conversation = convModel->getConversationForUID(LRCInstance::getCurrentConvUid());
    auto call = LRCInstance::getCallInfoForConversation(conversation);
    if (call)
        callId = call->id;

    /*
     * Decide whether a device has plugged, unplugged, or nothing has changed.
     */
    auto deviceList = avModel.getDevices();
    auto currentDeviceListSize = deviceList.size();

    DeviceEvent deviceEvent {DeviceEvent::None};
    if (currentDeviceListSize > deviceListSize_) {
        deviceEvent = DeviceEvent::Added;
    } else if (currentDeviceListSize < deviceListSize_) {
        /*
         * Check if the currentCaptureDevice is still in the device list.
         */
        if (std::find(std::begin(deviceList), std::end(deviceList), currentCaptureDevice)
            == std::end(deviceList)) {
            deviceEvent = DeviceEvent::RemovedCurrent;
        }
    }

    auto cb = [this, currentDeviceListSize, deviceEvent, defaultDevice, callId] {
        auto& avModel = LRCInstance::avModel();
        if (currentDeviceListSize == 0) {
            avModel.clearCurrentVideoCaptureDevice();
            avModel.switchInputTo({}, callId);
            avModel.stopPreview();
        } else if (deviceEvent == DeviceEvent::RemovedCurrent && currentDeviceListSize > 0) {
            avModel.setDefaultDevice(defaultDevice);
            avModel.setCurrentVideoCaptureDevice(defaultDevice);
            avModel.switchInputTo(defaultDevice, callId);
        }
    };

    if (LRCInstance::renderer()->isPreviewing()) {
        Utils::oneShotConnect(LRCInstance::renderer(),
                              &RenderManager::previewRenderingStopped,
                              [cb] { QtConcurrent::run([cb]() { cb(); }); });
    } else {
        if (deviceEvent == DeviceEvent::Added && currentDeviceListSize == 1) {
            avModel.setDefaultDevice(defaultDevice);
            avModel.setCurrentVideoCaptureDevice(defaultDevice);
            if (callId.isEmpty())
                LRCInstance::renderer()->startPreviewing();
            else
                avModel.switchInputTo(defaultDevice, callId);
        } else {
            cb();
        }
    }

    emit videoDeviceListChanged(currentDeviceListSize == 0);

    deviceListSize_ = currentDeviceListSize;
}