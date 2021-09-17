/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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

#pragma once

#include <QObject>

#include "qtutils.h"

class LRCInstance;

class CurrentAccountToMigrate : public QObject
{
    Q_OBJECT
    QML_RO_PROPERTY(int, accountToMigrateListSize)
    QML_RO_PROPERTY(QString, accountId)
    QML_RO_PROPERTY(QString, managerUsername)
    QML_RO_PROPERTY(QString, managerUri)
    QML_RO_PROPERTY(QString, username)
    QML_RO_PROPERTY(QString, alias)

public:
    explicit CurrentAccountToMigrate(LRCInstance* lrcInstance, QObject* parent = nullptr);
    ~CurrentAccountToMigrate();

    Q_INVOKABLE void removeCurrentAccountToMigrate();

Q_SIGNALS:
    void migrationEnded(bool success);
    void allMigrationsFinished();
    void currentAccountToMigrateRemoved();

private:
    void updateData();

    LRCInstance* lrcInstance_;

    // It will only be updated when starting to launch the client.
    QList<QString> accountToMigrateList_;

    QMetaObject::Connection migrationEndedConnection_;
};
