/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

#include "accountlistmodel.h"

#include <QDateTime>

#include "lrcinstance.h"
#include "utils.h"

AccountListModel::AccountListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

AccountListModel::~AccountListModel() {}

int
AccountListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        return LRCInstance::accountModel().getAccountList().size();
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
AccountListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
AccountListModel::data(const QModelIndex& index, int role) const
{
    auto accountList = LRCInstance::accountModel().getAccountList();
    if (!index.isValid() || accountList.size() <= index.row()) {
        return QVariant();
    }

    auto& accountInfo = LRCInstance::accountModel().getAccountInfo(accountList.at(index.row()));

    // Since we are using image provider right now, image url representation should be unique to
    // be able to use the image cache, account avatar will only be updated once PictureUid changed
    switch (role) {
    case Role::Alias:
        return QVariant(Utils::bestNameForAccount(accountInfo));
    case Role::Username:
        return QVariant(Utils::secondBestNameForAccount(accountInfo));
    case Role::Type:
        return QVariant(static_cast<int>(accountInfo.profileInfo.type));
    case Role::Status:
        return QVariant(static_cast<int>(accountInfo.status));
    case Role::ID:
        return QVariant(accountInfo.id);
    case Role::PictureUid:
        return avatarUidMap_[accountInfo.id];
    }
    return QVariant();
}

QHash<int, QByteArray>
AccountListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Alias] = "Alias";
    roles[Username] = "Username";
    roles[Type] = "Type";
    roles[Status] = "Status";
    roles[ID] = "ID";
    roles[PictureUid] = "PictureUid";
    return roles;
}

QModelIndex
AccountListModel::index(int row, int column, const QModelIndex& parent) const
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
AccountListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
AccountListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
AccountListModel::reset()
{
    beginResetModel();
    fillAvatarUidMap(LRCInstance::accountModel().getAccountList());
    endResetModel();
}

void
AccountListModel::updateAvatarUid(const QString& accountId)
{
    avatarUidMap_[accountId] = Utils::generateUid();
}

void
AccountListModel::fillAvatarUidMap(const QStringList& accountList)
{
    if (accountList.size() == 0) {
        avatarUidMap_.clear();
        return;
    }

    if (avatarUidMap_.isEmpty() || accountList.size() != avatarUidMap_.size()) {
        for (int i = 0; i < accountList.size(); ++i) {
            if (!avatarUidMap_.contains(accountList.at(i)))
                avatarUidMap_.insert(accountList.at(i), Utils::generateUid());
        }
    }
}
