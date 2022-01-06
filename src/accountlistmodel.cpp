/*
 * Copyright (C) 2019-2022 Savoir-faire Linux Inc.
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

#include "lrcinstance.h"
#include "utils.h"

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"

#include <QDateTime>

AccountListModel::AccountListModel(LRCInstance* instance, QObject* parent)
    : AbstractListModelBase(parent)
{
    lrcInstance_ = instance;
}

int
AccountListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        return lrcInstance_->accountModel().getAccountList().size();
    }
    return 0;
}

QVariant
AccountListModel::data(const QModelIndex& index, int role) const
{
    auto accountList = lrcInstance_->accountModel().getAccountList();
    if (!index.isValid() || accountList.size() <= index.row()) {
        return QVariant();
    }

    auto accountId = accountList.at(index.row());
    auto& accountInfo = lrcInstance_->accountModel().getAccountInfo(accountId);

    switch (role) {
    case Role::Alias:
        return QVariant(lrcInstance_->accountModel().bestNameForAccount(accountId));
    case Role::Username:
        return QVariant(lrcInstance_->accountModel().bestIdForAccount(accountId));
    case Role::Type:
        return QVariant(static_cast<int>(accountInfo.profileInfo.type));
    case Role::Status:
        return QVariant(static_cast<int>(accountInfo.status));
    case Role::ID:
        return QVariant(accountInfo.id);
    }
    return QVariant();
}

QHash<int, QByteArray>
AccountListModel::roleNames() const
{
    using namespace AccountList;
    QHash<int, QByteArray> roles;
#define X(role) roles[role] = #role;
    ACC_ROLES
#undef X
    return roles;
}

void
AccountListModel::reset()
{
    beginResetModel();
    endResetModel();
}
