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

#include "accountstomigratelistmodel.h"

AccountsToMigrateListModel::AccountsToMigrateListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

AccountsToMigrateListModel::~AccountsToMigrateListModel() {}

int
AccountsToMigrateListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        /*
         * Count.
         */
        auto accountList = LRCInstance::accountModel().getAccountList();

        int countAccountToMigrate = 0;

        for (const QString& i : accountList) {
            auto accountStatus = LRCInstance::accountModel().getAccountInfo(i).status;
            if (accountStatus == lrc::api::account::Status::ERROR_NEED_MIGRATION) {
                countAccountToMigrate++;
            }
        }

        return countAccountToMigrate;
    }
    /*
     * A valid QModelIndex returns 0 as no entry has sub-elements.
     */
    return 0;
}

int
AccountsToMigrateListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    /*
     * Only need one column.
     */
    return 1;
}

QVariant
AccountsToMigrateListModel::data(const QModelIndex& index, int role) const
{
    auto accountList = LRCInstance::accountModel().getAccountList();
    if (!index.isValid() || accountList.size() <= index.row()) {
        return QVariant();
    }

    QList<QString> accountToMigrateList;

    for (QString i : accountList) {
        auto accountStatus = LRCInstance::accountModel().getAccountInfo(i).status;
        if (accountStatus == lrc::api::account::Status::ERROR_NEED_MIGRATION) {
            accountToMigrateList.append(i);
        }
    }

    QString accountId = accountToMigrateList.at(index.row());

    auto& avatarInfo = LRCInstance::accountModel().getAccountInfo(accountId);

    switch (role) {
    case Role::Account_ID:
        return QVariant(accountId);
    case Role::ManagerUsername:
        return QVariant(avatarInfo.confProperties.managerUsername);
    case Role::ManagerUri:
        return QVariant(avatarInfo.confProperties.managerUri);
    case Role::Username:
        return QVariant(avatarInfo.confProperties.username);
    case Role::Alias:
        return QVariant(LRCInstance::accountModel().getAccountInfo(accountId).profileInfo.alias);
    case Role::Picture:
        return QString::fromLatin1(
            Utils::QImageToByteArray(Utils::accountPhoto(avatarInfo)).toBase64().data());
    }
    return QVariant();
}

QHash<int, QByteArray>
AccountsToMigrateListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Account_ID] = "Account_ID";
    roles[ManagerUsername] = "ManagerUsername";
    roles[ManagerUri] = "ManagerUri";
    roles[Username] = "Username";
    roles[Alias] = "Alias";
    roles[Picture] = "Picture";
    return roles;
}

QModelIndex
AccountsToMigrateListModel::index(int row, int column, const QModelIndex& parent) const
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
AccountsToMigrateListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

Qt::ItemFlags
AccountsToMigrateListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return flags;
}

void
AccountsToMigrateListModel::reset()
{
    beginResetModel();
    endResetModel();
}
