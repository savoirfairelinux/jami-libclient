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
    QML_RO_PROPERTY(lrc::api::video::DeviceType, currentRenderingDeviceType)
    QML_PROPERTY(bool, muteCamera)
    QML_RO_PROPERTY(QStringList, windowsNames)
    QML_RO_PROPERTY(QList<QVariant>, windowsIds)

public:
    explicit AvAdapter(LRCInstance* instance, QObject* parent = nullptr);
    ~AvAdapter() = default;

Q_SIGNALS:
    void screenCaptured(int screenNumber, QString source);
    // TODO: move to future audio device class
    void audioDeviceListChanged(int inputs, int outputs);

protected:
    void safeInit() override {};

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

    // Select window to display.
    Q_INVOKABLE void shareWindow(const QString& windowId);

    // Returns the screensharing resource
    Q_INVOKABLE QString getSharingResource(int screenId, const QString& key);

    Q_INVOKABLE void getListWindows();

    // Stop sharing the screen or file
    Q_INVOKABLE void stopSharing();

    Q_INVOKABLE void startAudioMeter();
    Q_INVOKABLE void stopAudioMeter();

    Q_INVOKABLE void setDeviceName(const QString& deviceName);

    Q_INVOKABLE void enableCodec(unsigned int id, bool isToEnable);
    Q_INVOKABLE void increaseCodecPriority(unsigned int id, bool isVideo);
    Q_INVOKABLE void decreaseCodecPriority(unsigned int id, bool isVideo);

    // TODO: to be removed
    Q_INVOKABLE bool getHardwareAcceleration();
    Q_INVOKABLE void setHardwareAcceleration(bool accelerate);

private Q_SLOTS:
    void onAudioDeviceEvent();

private:
    // Get screens arrangement rect relative to primary screen.
    const QRect getAllScreensBoundingRect();

    // Get the screen number
    int getScreenNumber() const;
};
