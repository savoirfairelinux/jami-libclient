/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
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

#include "lrcinstance.h"

#include <QSystemTrayIcon>

class GlobalSystemTray final : public QSystemTrayIcon
{
    Q_OBJECT

public:
    ~GlobalSystemTray() = default;
    static GlobalSystemTray& instance()
    {
        static GlobalSystemTray* instance_ = new GlobalSystemTray();
        return *instance_;
    }

    template<typename Func>
    static void connectClicked(Func&& onClicked)
    {
        auto& instance_ = instance();
        instance_.disconnect(instance_.messageClicked_);
        instance_.connect(&instance_, &QSystemTrayIcon::messageClicked, onClicked);
    }

private:
    explicit GlobalSystemTray()
        : QSystemTrayIcon() {};

    QMetaObject::Connection messageClicked_;
};
