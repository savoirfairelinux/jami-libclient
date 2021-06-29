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

#include <QObject>
#include <QMap>

class LRCInstance;

class AvatarRegistry : public QObject
{
    Q_OBJECT
public:
    explicit AvatarRegistry(LRCInstance* instance, QObject* parent = nullptr);
    ~AvatarRegistry() = default;

    // get a uid for an image in the cache
    Q_INVOKABLE QString getUid(const QString& id);

    // add or update a specific image in the cache
    QString addOrUpdateImage(const QString& id);

Q_SIGNALS:
    void avatarUidChanged(const QString& id);

private Q_SLOTS:
    void connectAccount();
    void onProfileUpdated(const QString& uri);

private:
    // Used to force cache updates via QQuickImageProvider
    QMap<QString, QString> uidMap_;

    LRCInstance* lrcInstance_;
};
