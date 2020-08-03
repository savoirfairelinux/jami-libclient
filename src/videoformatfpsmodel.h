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

#include <QAbstractItemModel>

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/newdevicemodel.h"

#include "lrcinstance.h"

class VideoFormatFpsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentResolution READ getCurrentResolution WRITE setCurrentResolution NOTIFY
                   currentResolutionChanged);

public:
    enum Role { FPS = Qt::UserRole + 1, FPS_ToDisplay_UTF8 };
    Q_ENUM(Role)

    explicit VideoFormatFpsModel(QObject *parent = 0);
    ~VideoFormatFpsModel();

    /*
     * QAbstractListModel override.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /*
     * Override role name as access point in qml.
     */
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /*
     * This function is to reset the model when there's new account added.
     */
    Q_INVOKABLE void reset();
    /*
     * This function is to get the current device id in the demon.
     */
    Q_INVOKABLE int getCurrentSettingIndex();

    /*
     * Getters and setters
     */
    QString getCurrentResolution();
    void setCurrentResolution(QString resNew);

signals:
    void currentResolutionChanged(QString resNew);

private:
    QString currentResolution_;
};
