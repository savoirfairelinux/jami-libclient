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

#include "qmladapterbase.h"
#include "lrcinstance.h"

#include <QObject>
#include <QVariant>
#include <QString>
#include <qtutils.h>

class AvAdapter final : public QmlAdapterBase
{
    Q_OBJECT
    QML_PROPERTY(lrc::api::video::DeviceType, currentRenderingDeviceType)

public:
    explicit AvAdapter(LRCInstance* instance, QObject* parent = nullptr);
    ~AvAdapter() = default;

Q_SIGNALS:

    // Emitted when the size of the video capture device list changes.
    void videoDeviceListChanged(bool listIsEmpty);

    void screenCaptured(int screenNumber, QString source);

protected:
    void safeInit() override {};

    // switch preview video input by device name
    Q_INVOKABLE void selectVideoInputDeviceByName(const QString& deviceName);

    // switch preview video input by device id
    Q_INVOKABLE void selectVideoInputDeviceById(const QString& deviceId);

    // Share the screen specificed by screen number.
    Q_INVOKABLE void shareEntireScreen(int screenNumber);

    // Share the all screens connected.
    Q_INVOKABLE void shareAllScreens();

    // Take snap shot of the screen and return emitting signal.
    Q_INVOKABLE void captureScreen(int screenNumber);

    // Take snap shot of the all screens and return by emitting signal.
    Q_INVOKABLE void captureAllScreens();

    // Share a media file.
    Q_INVOKABLE void shareFile(const QString& filePath);

    // Select screen area to display (from all screens).
    Q_INVOKABLE void shareScreenArea(unsigned x, unsigned y, unsigned width, unsigned height);

    // Stop sharing the screen or file
    Q_INVOKABLE void stopSharing();

    Q_INVOKABLE void startAudioMeter(bool async);
    Q_INVOKABLE void stopAudioMeter(bool async);

private:
    // Get screens arrangement rect relative to primary screen.
    const QRect getAllScreensBoundingRect();

    // Get current callId from current selected conv id.
    QString getCurrentCallId();

    // Used to classify capture device events.
    enum class DeviceEvent { Added, RemovedCurrent, None };

    // Used to track the capture device count.
    int deviceListSize_;

    // Device changed slot.
    void slotDeviceEvent();

    // Get the screen number
    int getScreenNumber() const;
};
