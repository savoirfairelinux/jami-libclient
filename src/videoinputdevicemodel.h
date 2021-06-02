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

#pragma once

#include "abstractlistmodelbase.h"

class VideoInputDeviceModel : public AbstractListModelBase
{
    Q_OBJECT
public:
    enum Role {
        DeviceName = Qt::UserRole + 1,
        DeviceChannel,
        DeviceId,
        CurrentFrameRate,
        CurrentResolution,
        DeviceName_UTF8,
        isCurrent
    };
    Q_ENUM(Role)

    explicit VideoInputDeviceModel(QObject* parent = nullptr);
    ~VideoInputDeviceModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reset();
    Q_INVOKABLE int deviceCount();

    // get model index of the current device
    Q_INVOKABLE int getCurrentIndex() const;
};
