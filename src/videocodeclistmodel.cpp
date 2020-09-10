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

#include "videocodeclistmodel.h"

#include <QList>

VideoCodecListModel::VideoCodecListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

VideoCodecListModel::~VideoCodecListModel() {}

int
VideoCodecListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        QList<lrc::api::Codec> realCodecList;
        auto videoCodecListOld = LRCInstance::getCurrentAccountInfo().codecModel->getVideoCodecs();

        for (auto codec : videoCodecListOld) {
            if (codec.name.length()) {
                realCodecList.append(codec);
            }
        }

        return realCodecList.size();
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
VideoCodecListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
VideoCodecListModel::data(const QModelIndex& index, int role) const
{
    auto videoCodecList = LRCInstance::getCurrentAccountInfo().codecModel->getVideoCodecs();
    if (!index.isValid() || videoCodecList.size() <= index.row()) {
        return QVariant();
    }

    QList<lrc::api::Codec> realCodecList;

    for (auto codec : videoCodecList) {
        if (codec.name.length()) {
            realCodecList.append(codec);
        }
    }

    switch (role) {
    case Role::VideoCodecName:
        return QVariant(realCodecList.at(index.row()).name);
    case Role::IsEnabled:
        return QVariant(realCodecList.at(index.row()).enabled);
    case Role::VideoCodecID:
        return QVariant(realCodecList.at(index.row()).id);
    }
    return QVariant();
}

QHash<int, QByteArray>
VideoCodecListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[VideoCodecName] = "VideoCodecName";
    roles[IsEnabled] = "IsEnabled";
    roles[VideoCodecID] = "VideoCodecID";
    return roles;
}

QModelIndex
VideoCodecListModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (column != 0) {
        return QModelIndex();
    }

    if (row >= 0 && row < rowCount()) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex
VideoCodecListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
VideoCodecListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable
                 | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}
