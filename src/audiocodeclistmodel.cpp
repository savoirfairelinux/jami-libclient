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

#include "audiocodeclistmodel.h"

AudioCodecListModel::AudioCodecListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

AudioCodecListModel::~AudioCodecListModel() {}

int
AudioCodecListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return LRCInstance::getCurrentAccountInfo().codecModel->getAudioCodecs().size();
    }
    return 0;
}

int
AudioCodecListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
AudioCodecListModel::data(const QModelIndex& index, int role) const
{
    auto audioCodecList = LRCInstance::getCurrentAccountInfo().codecModel->getAudioCodecs();
    if (!index.isValid() || audioCodecList.size() <= index.row()) {
        return QVariant();
    }

    switch (role) {
    case Role::AudioCodecName:
        return QVariant(audioCodecList.at(index.row()).name);
    case Role::IsEnabled:
        return QVariant(audioCodecList.at(index.row()).enabled);
    case Role::AudioCodecID:
        return QVariant(audioCodecList.at(index.row()).id);
    case Role::Samplerate:
        return QVariant(audioCodecList.at(index.row()).samplerate);
    }
    return QVariant();
}

QHash<int, QByteArray>
AudioCodecListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[AudioCodecName] = "AudioCodecName";
    roles[IsEnabled] = "IsEnabled";
    roles[AudioCodecID] = "AudioCodecID";
    roles[Samplerate] = "Samplerate";
    return roles;
}

QModelIndex
AudioCodecListModel::index(int row, int column, const QModelIndex& parent) const
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
AudioCodecListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
AudioCodecListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable
                 | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}
