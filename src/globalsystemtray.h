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

class GlobalSystemTray : public QSystemTrayIcon
{
    Q_OBJECT

public:
    static GlobalSystemTray &
    instance()
    {
        static GlobalSystemTray *instance_ = new GlobalSystemTray();
        return *instance_;
    }

    /*
     * Remember the last triggering account for the notification,
     * safe since user cannot activate previous notifications.
     */
    void setTriggeredAccountId(const QString &accountId);

    const QString &getTriggeredAccountId();

    void setPossibleOnGoingConversationInfo(const lrc::api::conversation::Info &convInfo);

    const lrc::api::conversation::Info &getPossibleOnGoingConversationInfo();

private:
    GlobalSystemTray();

    QString triggeredAccountId_;
    lrc::api::conversation::Info triggeredOnGoingConvInfo_;
};
