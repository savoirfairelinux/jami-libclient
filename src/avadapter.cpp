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

#ifdef Q_OS_LINUX
#include "xrectsel.h"
#endif

#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QPainter>
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
    auto deviceId = LRCInstance::avModel().getDeviceIdFromName(deviceName);
    if (deviceId.isEmpty()) {
        qWarning() << "Couldn't find device: " << deviceName;
        return;
    }
    LRCInstance::avModel().setCurrentVideoCaptureDevice(deviceId);
    LRCInstance::avModel().switchInputTo(deviceId, getCurrentCallId());
}

void
AvAdapter::shareEntireScreen(int screenNumber)
{
    QScreen* screen = QGuiApplication::screens().at(screenNumber);
    if (!screen)
        return;
    QRect rect = screen->geometry();

    LRCInstance::avModel()
        .setDisplay(getScreenNumber(), rect.x(), rect.y(), rect.width(), rect.height(), getCurrentCallId());
}

void
AvAdapter::shareAllScreens()
{
    auto screens = QGuiApplication::screens();

    int width = 0, height = 0;
    for (auto scr : screens) {
        width += scr->geometry().width();
        if (height < scr->geometry().height())
            height = scr->geometry().height();
    }

    LRCInstance::avModel().setDisplay(getScreenNumber(), 0, 0, width, height, getCurrentCallId());
}

void
AvAdapter::captureScreen(int screenNumber)
{
    QtConcurrent::run([this, screenNumber]() {
        QScreen* screen = QGuiApplication::screens().at(screenNumber);
        if (!screen)
            return;
        /*
         * The screen window id is always 0.
         */
        auto pixmap = screen->grabWindow(0);

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");

        emit screenCaptured(screenNumber, Utils::byteArrayToBase64String(buffer.data()));
    });
}

void
AvAdapter::captureAllScreens()
{
    QtConcurrent::run([this]() {
        auto screens = QGuiApplication::screens();

        QList<QPixmap> scrs;
        int width = 0, height = 0, currentPoint = 0;

        foreach (auto scr, screens) {
            QPixmap pix = scr->grabWindow(0);
            width += pix.width();
            if (height < pix.height())
                height = pix.height();
            scrs << pix;
        }

        QPixmap final(width, height);
        QPainter painter(&final);
        final.fill(Qt::black);

        foreach (auto scr, scrs) {
            painter.drawPixmap(QPoint(currentPoint, 0), scr);
            currentPoint += scr.width();
        }

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        final.save(&buffer, "PNG");
        emit screenCaptured(-1, Utils::byteArrayToBase64String(buffer.data()));
    });
}

void
AvAdapter::shareFile(const QString& filePath)
{
    LRCInstance::avModel().setInputFile(filePath, getCurrentCallId());
}

void
AvAdapter::shareScreenArea(unsigned x, unsigned y, unsigned width, unsigned height)
{
#ifdef Q_OS_LINUX
    // xrectsel will freeze all displays too fast so that the call
    // context menu will not be closed even closed signal is emitted
    // use timer to wait until popup is closed
    QTimer::singleShot(100, [=]() mutable {
        x = y = width = height = 0;
        xrectsel(&x, &y, &width, &height);

        LRCInstance::avModel().setDisplay(getScreenNumber(),
                                          x,
                                          y,
                                          width < 128 ? 128 : width,
                                          height < 128 ? 128 : height,
                                          getCurrentCallId());
    });
#else
    LRCInstance::avModel().setDisplay(getScreenNumber(),
                                      x,
                                      y,
                                      width < 128 ? 128 : width,
                                      height < 128 ? 128 : height,
                                      getCurrentCallId());
#endif
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

const QString&
AvAdapter::getCurrentCallId()
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(LRCInstance::getCurrentConvUid());
    auto call = LRCInstance::getCallInfoForConversation(convInfo);
    if (!call)
        return QString();
    return call->id;
}

void
AvAdapter::slotDeviceEvent()
{
    auto& avModel = LRCInstance::avModel();
    auto defaultDevice = avModel.getDefaultDevice();
    auto currentCaptureDevice = avModel.getCurrentVideoCaptureDevice();
    QString callId = getCurrentCallId();

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

int
AvAdapter::getScreenNumber() const
{
    int display = 0;

#ifdef Q_OS_LINUX
    // Get display
    QString display_env {getenv("DISPLAY")};
    if (!display_env.isEmpty()) {
        auto list = display_env.split(":", Qt::SkipEmptyParts);
        // Should only be one display, so get the first one
        if (list.size() > 0) {
            display = list.at(0).toInt();
        }
    }
#endif
    return display;
}
