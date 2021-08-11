/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#include <QSortFilterProxyModel>

class DeviceItemListModel : public AbstractListModelBase
{
    Q_OBJECT

public:
    enum Role { DeviceName = Qt::UserRole + 1, DeviceID, IsCurrent };
    Q_ENUM(Role)

    explicit DeviceItemListModel(QObject* parent = nullptr);
    ~DeviceItemListModel();

    // QAbstractListModel override.
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Override role name as access point in qml.
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& child) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

    // This function is to reset the model when there's new account added.
    Q_INVOKABLE void reset();

    Q_INVOKABLE void revokeDevice(QString deviceId, QString password);

public Q_SLOTS:
    void onAccountChanged();
};

class DeviceItemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(LRCInstance* lrcInstance READ getLrcInstance WRITE setLrcInstance)

public:
    explicit DeviceItemProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        sourceModel_ = new DeviceItemListModel(this);

        setSourceModel(sourceModel_);
        setSortRole(DeviceItemListModel::Role::IsCurrent);
        sort(0, Qt::DescendingOrder);
        setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    }

    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
    {
        QVariant leftIsCurrent = sourceModel()->data(left, DeviceItemListModel::Role::IsCurrent);
        QVariant rightIsCurrent = sourceModel()->data(right, DeviceItemListModel::Role::IsCurrent);

        if (leftIsCurrent.toBool())
            return false;
        if (rightIsCurrent.toBool())
            return true;

        QChar leftDeviceNameFirstChar
            = sourceModel()->data(left, DeviceItemListModel::Role::DeviceName).toString().at(0);
        QChar rightDeviceNameFirstChar
            = sourceModel()->data(right, DeviceItemListModel::Role::DeviceName).toString().at(0);

        return leftDeviceNameFirstChar < rightDeviceNameFirstChar;
    }

    LRCInstance* getLrcInstance()
    {
        return sourceModel_->property("lrcInstance").value<LRCInstance*>();
    }

    void setLrcInstance(LRCInstance* instance)
    {
        sourceModel_->setProperty("lrcInstance", QVariant::fromValue(instance));
    }

private:
    DeviceItemListModel* sourceModel_;
};