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

#ifdef Q_OS_LINUX
enum class NotificationType { INVALID, CALL, REQUEST, CHAT };
Q_ENUMS(NotificationType)
#endif // Q_OS_LINUX

class AppSettingsManager;

class SystemTray final : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTray(AppSettingsManager* settingsManager, QObject* parent = nullptr);
    ~SystemTray();

#ifdef Q_OS_LINUX
    bool hideNotification(const QString& id);
    void showNotification(const QString& id,
                          const QString& title,
                          const QString& body,
                          NotificationType type,
                          const QByteArray& avatar = {});

Q_SIGNALS:
    void openConversationActivated(const QString& accountId, const QString& convUid);
    void acceptPendingActivated(const QString& accountId, const QString& peerUri);
    void refusePendingActivated(const QString& accountId, const QString&);
    void answerCallActivated(const QString& accountId, const QString&);
    void declineCallActivated(const QString& accountId, const QString&);
#else
    void showNotification(const QString& message,
                          const QString& from,
                          std::function<void()> const& onClickedCb);

    template<typename Func>
    void setOnClickedCallback(Func&& onClickedCb);
#endif // Q_OS_LINUX

private:
    QMetaObject::Connection messageClicked_;
    AppSettingsManager* settingsManager_;

    struct SystemTrayImpl;
    std::unique_ptr<SystemTrayImpl> pimpl_;
};
