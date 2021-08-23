/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "currentaccount.h"

CurrentAccount::CurrentAccount(LRCInstance* lrcInstance, QObject* parent)
    : QObject(parent)
    , lrcInstance_(lrcInstance)
{
    connect(&lrcInstance_->accountModel(),
            &NewAccountModel::accountStatusChanged,
            this,
            &CurrentAccount::onAccountUpdated);

    connect(&lrcInstance_->accountModel(),
            &NewAccountModel::profileUpdated,
            this,
            &CurrentAccount::onAccountUpdated);

    connect(lrcInstance_, &LRCInstance::currentAccountIdChanged, [this] { updateData(); });
    updateData();
}

void
CurrentAccount::onAccountUpdated(const QString& id)
{
    // filter for our currently set id
    if (id_ != id)
        return;
    updateData();
}

void
CurrentAccount::updateData()
{
    set_id(lrcInstance_->get_currentAccountId());
    try {
        const auto& accInfo = lrcInstance_->getAccountInfo(id_);
        set_uri(accInfo.profileInfo.uri);
        set_registeredName(accInfo.registeredName);
        set_alias(accInfo.profileInfo.alias);
        set_bestId(lrcInstance_->accountModel().bestIdForAccount(id_));
        set_bestName(lrcInstance_->accountModel().bestNameForAccount(id_));
        set_hasAvatarSet(!accInfo.profileInfo.avatar.isEmpty());
        set_status(accInfo.status);
        set_type(accInfo.profileInfo.type);
    } catch (...) {
        qWarning() << "Can't update current account data for" << id_;
    }
}
