/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gonsimsantos@savoirfairelinux.com>
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

#include "mediacodeclistmodel.h"

#include "lrcinstance.h"

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/newcodecmodel.h"
#include "api/newdevicemodel.h"

MediaCodecListModel::MediaCodecListModel(QObject* parent)
    : AbstractListModelBase(parent)
{
    connect(this, &MediaCodecListModel::lrcInstanceChanged, [this]() {
        connect(lrcInstance_,
                &LRCInstance::currentAccountIdChanged,
                this,
                &MediaCodecListModel::reset);
    });
}

MediaCodecListModel::~MediaCodecListModel() {}

int
MediaCodecListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        if (mediaType_ == MediaCodecListModel::MediaType::AUDIO)
            return lrcInstance_->getCurrentAccountInfo().codecModel->getAudioCodecs().size();
        if (mediaType_ == MediaCodecListModel::MediaType::VIDEO) {
            QList<lrc::api::Codec> realCodecList;
            auto videoCodecListOld = lrcInstance_->getCurrentAccountInfo()
                                         .codecModel->getVideoCodecs();

            for (auto codec : videoCodecListOld) {
                if (codec.name.length()) {
                    realCodecList.append(codec);
                }
            }

            return realCodecList.size();
        }
    }
    return 0;
}

int
MediaCodecListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
MediaCodecListModel::data(const QModelIndex& index, int role) const
{
    QList<lrc::api::Codec> mediaCodecList;
    if (mediaType_ == MediaCodecListModel::MediaType::AUDIO)
        mediaCodecList = lrcInstance_->getCurrentAccountInfo().codecModel->getAudioCodecs();
    else if (mediaType_ == MediaCodecListModel::MediaType::VIDEO) {
        QList<lrc::api::Codec> videoCodecList = lrcInstance_->getCurrentAccountInfo()
                                                    .codecModel->getVideoCodecs();

        for (auto codec : videoCodecList) {
            if (codec.name.length()) {
                mediaCodecList.append(codec);
            }
        }
    }
    if (!index.isValid() || mediaCodecList.size() <= index.row()) {
        return QVariant();
    }

    switch (role) {
    case Role::MediaCodecName:
        return QVariant(mediaCodecList.at(index.row()).name);
    case Role::IsEnabled:
        return QVariant(mediaCodecList.at(index.row()).enabled);
    case Role::MediaCodecID:
        return QVariant(mediaCodecList.at(index.row()).id);
    case Role::Samplerate:
        return QVariant(mediaCodecList.at(index.row()).samplerate);
    }
    return QVariant();
}

QHash<int, QByteArray>
MediaCodecListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MediaCodecName] = "MediaCodecName";
    roles[IsEnabled] = "IsEnabled";
    roles[MediaCodecID] = "MediaCodecID";
    roles[Samplerate] = "Samplerate";
    return roles;
}

QModelIndex
MediaCodecListModel::index(int row, int column, const QModelIndex& parent) const
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
MediaCodecListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
MediaCodecListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable
                 | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
MediaCodecListModel::reset()
{
    beginResetModel();
    endResetModel();
}

int
MediaCodecListModel::mediaType()
{
    return mediaType_;
}

void
MediaCodecListModel::setMediaType(int mediaType)
{
    mediaType_ = mediaType;
}
