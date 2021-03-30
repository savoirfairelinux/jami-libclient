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

#include "systemtray.h"

#include "appsettingsmanager.h"

SystemTray::SystemTray(AppSettingsManager* settingsManager, QObject* parent)
    : QSystemTrayIcon(parent)
    , settingsManager_(settingsManager)
{}

SystemTray::~SystemTray()
{
    hide();
}

void
SystemTray::showNotification(const QString& message,
                             const QString& from,
                             std::function<void()> const& onClickedCb)
{
    if (!settingsManager_->getValue(Settings::Key::EnableNotifications).toBool()) {
        qWarning() << "Notifications are disabled";
        return;
    }

    setOnClickedCallback(std::move(onClickedCb));

    if (from.isEmpty())
        showMessage(message, "", QIcon(":images/jami.png"));
    else
        showMessage(from, message, QIcon(":images/jami.png"));
}

template<typename Func>
void
SystemTray::setOnClickedCallback(Func&& onClicked)
{
    disconnect(messageClicked_);
    messageClicked_ = connect(this, &QSystemTrayIcon::messageClicked, onClicked);
}
