/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

class AudioDeviceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Type { Invalid, Record, Playback, Ringtone };
    Q_ENUM(Type)
    Q_PROPERTY(Type type MEMBER type_ NOTIFY typeChanged)

    enum Role { DeviceName = Qt::UserRole + 1, RawDeviceName };
    Q_ENUM(Role)

signals:
    void typeChanged();

public:
    explicit AudioDeviceModel(QObject* parent = 0);
    ~AudioDeviceModel() = default;

    /*
     * QAbstractListModel override.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /*
     * Override role name as access point in qml.
     */
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row,
                      int column = 0,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Q_INVOKABLE void reset();
    Q_INVOKABLE int getCurrentIndex();

private:
    QVector<QString> devices_;

    Type type_ {Type::Invalid};
};
