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

#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"

AccountListModel::AccountListModel(LRCInstance* instance, QObject* parent)
    : AbstractListModelBase(parent)
{
    lrcInstance_ = instance;
}

AccountListModel::~AccountListModel() {}

int
AccountListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid() && lrcInstance_) {
        /*
         * Count.
         */
        return lrcInstance_->accountModel().getAccountList().size();
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
    auto accountList = lrcInstance_->accountModel().getAccountList();
    if (!index.isValid() || accountList.size() <= index.row()) {
        return QVariant();
    }

    auto accountId = accountList.at(index.row());
    auto& accountInfo = lrcInstance_->accountModel().getAccountInfo(accountId);

    // Since we are using image provider right now, image url representation should be unique to
    // be able to use the image cache, account avatar will only be updated once PictureUid changed
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
    case Role::PictureUid:
        return avatarUidMap_[accountInfo.id];
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
    fillAvatarUidMap(lrcInstance_->accountModel().getAccountList());
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
