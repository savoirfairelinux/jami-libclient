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

#pragma once

#include "lrcinstance.h"

#include <QObject>
#include <QString>

class CurrentAccount final : public QObject
{
    Q_OBJECT
    QML_RO_PROPERTY(QString, id)
    QML_RO_PROPERTY(QString, uri)
    QML_RO_PROPERTY(QString, registeredName)
    QML_RO_PROPERTY(QString, alias)
    QML_RO_PROPERTY(QString, bestId)
    QML_RO_PROPERTY(QString, bestName)
    QML_RO_PROPERTY(bool, hasAvatarSet)
    QML_RO_PROPERTY(lrc::api::account::Status, status)
    QML_RO_PROPERTY(lrc::api::profile::Type, type)

public:
    explicit CurrentAccount(LRCInstance* lrcInstance, QObject* parent = nullptr);
    ~CurrentAccount() = default;

private Q_SLOTS:
    void updateData();
    void onAccountUpdated(const QString& id);

private:
    LRCInstance* lrcInstance_;
};
