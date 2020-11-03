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

#include <QObject>
#include <QVariant>
#include <QString>

class AvAdapter final : public QmlAdapterBase
{
    Q_OBJECT

public:
    explicit AvAdapter(QObject* parent = nullptr);
    ~AvAdapter() = default;

signals:

    /*
     * Emitted when the size of the video capture device list changes.
     */
    void videoDeviceListChanged();

protected:
    void safeInit() override {};

    /*
     * Return needed info for populating video device context menu item.
     */
    Q_INVOKABLE QVariantMap populateVideoDeviceContextMenuItem();

    /*
     * Preview video input switching.
     */
    Q_INVOKABLE void onVideoContextMenuDeviceItemClicked(const QString& deviceName);

    /*
     * Share the screen specificed by screen number.
     */
    Q_INVOKABLE void shareEntireScreen(int screenNumber);

    /*
     * Take snap shot of the screen by returning base64 image string.
     */
    Q_INVOKABLE const QString captureScreen(int screenNumber);

    /*
     * Share a media file.
     */
    Q_INVOKABLE void shareFile(const QString& filePath);

    /*
     * Select screen area to display.
     */
    Q_INVOKABLE void shareScreenArea(int screenNumber, int x, int y, int width, int height);

    Q_INVOKABLE void startAudioMeter(bool async);
    Q_INVOKABLE void stopAudioMeter(bool async);

private:
    /*
     * Used to classify capture device events.
     */
    enum class DeviceEvent { Added, RemovedCurrent, None };

    /*
     * Used to track the capture device count.
     */
    int deviceListSize_;

    /*
     * Device changed slot.
     */
    void slotDeviceEvent();
};
