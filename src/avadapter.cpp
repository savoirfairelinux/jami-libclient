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
#include "qtutils.h"

#ifdef Q_OS_LINUX
#include "xrectsel.h"
#endif

#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QPainter>
#include <QScreen>

AvAdapter::AvAdapter(LRCInstance* instance, QObject* parent)
    : QmlAdapterBase(instance, parent)
{
    auto& avModel = lrcInstance_->avModel();

    deviceListSize_ = avModel.getDevices().size();
    connect(&avModel, &lrc::api::AVModel::audioDeviceEvent, this, &AvAdapter::onAudioDeviceEvent);
    connect(&avModel, &lrc::api::AVModel::deviceEvent, this, &AvAdapter::onVideoDeviceEvent);
    connect(lrcInstance_->renderer(), &RenderManager::previewFrameStarted, [this]() {
        // TODO: listen to the correct signals that are needed to be added in daemon or lrc
        auto callId = getCurrentCallId();
        if (!callId.isEmpty())
            set_currentRenderingDeviceType(
                lrcInstance_->avModel().getCurrentRenderedDevice(callId).type);
    });
}

void
AvAdapter::selectVideoInputDeviceByName(const QString& deviceName)
{
    auto deviceId = lrcInstance_->avModel().getDeviceIdFromName(deviceName);
    if (deviceId.isEmpty()) {
        qWarning() << "Couldn't find device: " << deviceName;
        return;
    }
    selectVideoInputDeviceById(deviceId);
}

void
AvAdapter::selectVideoInputDeviceById(const QString& deviceId)
{
    lrcInstance_->avModel().setCurrentVideoCaptureDevice(deviceId);
    lrcInstance_->avModel().switchInputTo(deviceId, getCurrentCallId());
}

// The top left corner of primary screen is (0, 0).
// For Qt, QScreen geometry contains x, y location relative to primary screen.
// The purpose of the function is to use calculate a boundingRect for virtual desktop
// to help screen sharing.
const QRect
AvAdapter::getAllScreensBoundingRect()
{
    auto screens = QGuiApplication::screens();

    // p0 is for x axis, p1 is for y axis,
    // points contain values that are the maximum positive and negative domain value
    QPoint p0(0, 0), p1(0, 0);

    for (auto scr : screens) {
        auto devicePixelRatio = scr->devicePixelRatio();
        auto screenRect = scr->geometry();

        if (screenRect.y() < 0 && p1.y() < abs(screenRect.y()))
            p1.setY(abs(screenRect.y()));
        else if (screenRect.y() >= 0
                 && p1.x() < screenRect.y() + screenRect.height() * devicePixelRatio)
            p1.setX(screenRect.y() + screenRect.height() * devicePixelRatio);

        if (screenRect.x() < 0 && p0.y() < abs(screenRect.x()))
            p0.setY(abs(screenRect.x()));
        else if (screenRect.x() >= 0
                 && p0.x() < screenRect.x() + screenRect.width() * devicePixelRatio)
            p0.setX(screenRect.x() + screenRect.width() * devicePixelRatio);
    }

    return QRect(-p0.y(), -p1.y(), p0.y() + p0.x(), p1.y() + p1.x());
}

void
AvAdapter::shareEntireScreen(int screenNumber)
{
    QScreen* screen = QGuiApplication::screens().at(screenNumber);
    if (!screen)
        return;
    QRect rect = screen->geometry();

    lrcInstance_->avModel().setDisplay(getScreenNumber(),
                                       rect.x(),
                                       rect.y(),
                                       rect.width() * screen->devicePixelRatio(),
                                       rect.height() * screen->devicePixelRatio(),
                                       getCurrentCallId());
}

void
AvAdapter::shareAllScreens()
{
    const auto arrangementRect = getAllScreensBoundingRect();

    lrcInstance_->avModel().setDisplay(getScreenNumber(),
                                       arrangementRect.x(),
                                       arrangementRect.y(),
                                       arrangementRect.width(),
                                       arrangementRect.height(),
                                       getCurrentCallId());
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

        Q_EMIT screenCaptured(screenNumber, Utils::byteArrayToBase64String(buffer.data()));
    });
}

void
AvAdapter::captureAllScreens()
{
    QtConcurrent::run([this]() {
        auto screens = QGuiApplication::screens();

        QList<QPixmap> scrs;
        int width = 0, height = 0, currentPoint = 0;

        for (auto scr : screens) {
            QPixmap pix = scr->grabWindow(0);
            auto devicePixelRatio = scr->devicePixelRatio();
            width += scr->geometry().width() * devicePixelRatio;
            if (height < scr->geometry().height() * devicePixelRatio)
                height = scr->geometry().height() * devicePixelRatio;
            scrs << pix;
        }

        QPixmap final(width, height);
        QPainter painter(&final);
        final.fill(Qt::black);

        for (auto scr : scrs) {
            painter.drawPixmap(currentPoint, 0, scr.width(), scr.height(), scr);
            currentPoint += scr.width();
        }

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        final.save(&buffer, "PNG");
        Q_EMIT screenCaptured(-1, Utils::byteArrayToBase64String(buffer.data()));
    });
}

void
AvAdapter::shareFile(const QString& filePath)
{
    lrcInstance_->avModel().setInputFile(filePath, getCurrentCallId());
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

        lrcInstance_->avModel().setDisplay(getScreenNumber(),
                                           x,
                                           y,
                                           width < 128 ? 128 : width,
                                           height < 128 ? 128 : height,
                                           getCurrentCallId());
    });
#else
    lrcInstance_->avModel().setDisplay(getScreenNumber(),
                                       x,
                                       y,
                                       width < 128 ? 128 : width,
                                       height < 128 ? 128 : height,
                                       getCurrentCallId());
#endif
}

void
AvAdapter::stopSharing()
{
    auto callId = getCurrentCallId();
    if (!callId.isEmpty())
        lrcInstance_->avModel().switchInputTo(lrcInstance_->avModel().getCurrentVideoCaptureDevice(),
                                              callId);
}

void
AvAdapter::startAudioMeter()
{
    lrcInstance_->startAudioMeter();
}

void
AvAdapter::stopAudioMeter()
{
    lrcInstance_->stopAudioMeter();
}

void
AvAdapter::onAudioDeviceEvent()
{
    auto& avModel = lrcInstance_->avModel();
    auto inputs = avModel.getAudioInputDevices().size();
    auto outputs = avModel.getAudioOutputDevices().size();
    Q_EMIT audioDeviceListChanged(inputs, outputs);
}

void
AvAdapter::onVideoDeviceEvent()
{
    auto& avModel = lrcInstance_->avModel();
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
        auto& avModel = lrcInstance_->avModel();
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

    if (lrcInstance_->renderer()->isPreviewing()) {
        Utils::oneShotConnect(lrcInstance_->renderer(),
                              &RenderManager::previewRenderingStopped,
                              [cb] { QtConcurrent::run([cb]() { cb(); }); });
    } else {
        if (deviceEvent == DeviceEvent::Added && currentDeviceListSize == 1) {
            avModel.setDefaultDevice(defaultDevice);
            avModel.setCurrentVideoCaptureDevice(defaultDevice);
            if (callId.isEmpty()) {
                Q_EMIT videoDeviceAvailable();
            } else {
                avModel.switchInputTo(defaultDevice, callId);
            }
        } else {
            cb();
        }
    }

    Q_EMIT videoDeviceListChanged(currentDeviceListSize);

    deviceListSize_ = currentDeviceListSize;
}

QString
AvAdapter::getCurrentCallId()
{
    try {
        const auto& convInfo = lrcInstance_->getConversationFromConvUid(
            lrcInstance_->get_selectedConvUid());
        auto call = lrcInstance_->getCallInfoForConversation(convInfo);
        return call ? call->id : QString();
    } catch (...) {
        return QString();
    }
}

int
AvAdapter::getScreenNumber() const
{
    int display = 0;

#ifdef Q_OS_LINUX
    // Get display
    QString display_env {getenv("DISPLAY")};
    if (!display_env.isEmpty()) {
        auto list = display_env.split(':', QString::SplitBehavior::SkipEmptyParts);
        // Should only be one display, so get the first one
        if (list.size() > 0) {
            display = list.at(0).toInt();
        }
    }
#endif
    return display;
}
