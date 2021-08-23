/*!
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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

#ifdef Q_OS_LINUX
#include <QDBusInterface>
#include <QDBusReply>
#define N_SERVICES 3
#endif

class ScreenSaver : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSaver(QObject* parent = nullptr);
    virtual ~ScreenSaver() = default;

    bool inhibit(void);
    bool uninhibit(void);
    bool isInhibited(void);

#ifdef Q_OS_LINUX
private:
    bool createInterface(void);
    QString services_[N_SERVICES] = { "org.freedesktop.ScreenSaver",
                                      "org.gnome.ScreenSaver",
                                      "org.mate.ScreenSaver" };

    QString paths_[N_SERVICES] = { "/org/freedesktop/ScreenSaver",
                                   "/org/gnome/ScreenSaver",
                                   "/org/mate/ScreenSaver" };

    uint request_;
    QDBusConnection sessionBus_;
    QDBusInterface* screenSaverInterface_;
#endif
};
