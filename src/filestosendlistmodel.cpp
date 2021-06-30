/*
 * Copyright (C) 2021 by Savoir-faire Linux
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

#include "filestosendlistmodel.h"

#include "utils.h"

#include <QFileInfo>
#include <QImageReader>

FilesToSendListModel::FilesToSendListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int
FilesToSendListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return pendingFiles_.size();
}

QHash<int, QByteArray>
FilesToSendListModel::roleNames() const
{
    using namespace FilesToSend;
    QHash<int, QByteArray> roles;
#define X(role) roles[role] = #role;
    FS_ROLES
#undef X
    return roles;
}

Q_INVOKABLE void
FilesToSendListModel::addToPending(QString filePath)
{
    auto fileInfo = QFileInfo(filePath);
    if (!fileInfo.exists())
        return;

    // QImageReader will treat .gz file (Jami archive) as svgz image format
    // so decideFormatFromContent is needed
    bool isImage = false;
    QImageReader reader;
    reader.setDecideFormatFromContent(true);
    reader.setFileName(filePath);

    if (!reader.read().isNull())
        isImage = true;

    beginInsertRows(QModelIndex(), pendingFiles_.size(), pendingFiles_.size());
    auto item = FilesToSend::Item(filePath, fileInfo.fileName(), isImage, fileInfo.size());
    pendingFiles_.append(item);
    endInsertRows();
}

void
FilesToSendListModel::removeFromPending(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    pendingFiles_.removeAt(index);
    endRemoveRows();
}

Q_INVOKABLE void
FilesToSendListModel::flush()
{
    beginRemoveRows(QModelIndex(), 0, pendingFiles_.size() - 1);
    pendingFiles_.clear();
    endRemoveRows();
}

QVariant
FilesToSendListModel::data(const QModelIndex& index, int role) const
{
    using namespace FilesToSend;

    auto item = pendingFiles_.at(index.row());

    switch (role) {
    case Role::FileName:
        return QVariant(item.fileName);
    case Role::FilePath:
        return QVariant(item.filePath);
    case Role::FileSize:
        return QVariant(Utils::humanFileSize(item.fileSizeInByte));
    case Role::IsImage:
        return QVariant(item.isImage);
    }
    return QVariant();
}
