/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

#define ACC_ROLES \
    X(Alias) \
    X(Username) \
    X(Type) \
    X(Status) \
    X(ID) \
    X(PictureUid)

namespace AccountList {
Q_NAMESPACE
enum Role {
    DummyRole = Qt::UserRole + 1,
#define X(role) role,
    ACC_ROLES
#undef X
};
Q_ENUM_NS(Role)
} // namespace AccountList

/*
 * The CurrentAccountFilterModel class
 * is for the sole purpose of filtering out current account.
 */
class CurrentAccountFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CurrentAccountFilterModel(LRCInstance* lrcInstance,
                                       QAbstractListModel* parent = nullptr)
        : QSortFilterProxyModel(parent)
        , lrcInstance_(lrcInstance)
    {
        setSourceModel(parent);
    }

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        // Accept all contacts in conversation list filtered with account type, except those in a call.
        auto index = sourceModel()->index(sourceRow, 0, sourceParent);
        auto accountId = sourceModel()->data(index, AccountList::ID);
        return accountId != lrcInstance_->get_currentAccountId();
    }

protected:
    LRCInstance* lrcInstance_ {nullptr};
};

class AccountListModel final : public AbstractListModelBase
{
    Q_OBJECT

public:
    explicit AccountListModel(LRCInstance* instance, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /*
     * This function is to reset the model when there's new account added.
     */
    Q_INVOKABLE void reset();

    /*
     * This function is to update avatar uuid when there's an avatar changed.
     */
    Q_INVOKABLE void updateAvatarUid(const QString& accountId);

protected:
    using Role = AccountList::Role;

private:
    /*
     * Give a uuid for each account avatar and it will serve PictureUid role
     */
    void fillAvatarUidMap(const QStringList& accountList);

    QMap<QString, QString> avatarUidMap_;
};
