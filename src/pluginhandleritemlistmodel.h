/**
 * Copyright (C) 2020-2022 Savoir-faire Linux Inc.
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

class LRCInstance;

class PluginHandlerItemListModel : public AbstractListModelBase
{
    Q_OBJECT

public:
    enum Role { HandlerName = Qt::UserRole + 1, HandlerId, HandlerIcon, IsLoaded, PluginId };
    Q_ENUM(Role)

    explicit PluginHandlerItemListModel(
        QObject* parent = nullptr,
        const QString& accountId = QString(""),
        const QString& peerId = QString(""),
        LRCInstance* instance = nullptr); // for calls, accountId is the callId and peerId is null
    ~PluginHandlerItemListModel();

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
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& child) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /*
     * This function is to reset the model when there's new account added.
     */
    Q_INVOKABLE void reset();

    QString callId();
    void setCallId(QString callId);
    QString accountId();
    void setAccountId(QString accountId);
    QString peerId();
    void setPeerId(QString peerId);

private:
    QString callId_ = QString("");
    QString accountId_ = QString("");
    QString peerId_ = QString("");
    bool isMediaHandler_;
};
