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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QSystemTrayIcon>

#include <functional>

class AppSettingsManager;

class SystemTray final : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTray(AppSettingsManager* settingsManager, QObject* parent = nullptr);
    ~SystemTray();

    void showNotification(const QString& message,
                          const QString& from,
                          std::function<void()> const& onClickedCb);

    template<typename Func>
    void setOnClickedCallback(Func&& onClickedCb);

private:
    QMetaObject::Connection messageClicked_;
    AppSettingsManager* settingsManager_;
};
