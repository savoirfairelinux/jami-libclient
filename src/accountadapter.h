/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#include "qmladapterbase.h"

#include <QObject>
#include <QSettings>
#include <QString>

#include "lrcinstance.h"
#include "utils.h"

class AppSettingsManager;

class AccountAdapter final : public QmlAdapterBase
{
    Q_OBJECT

    Q_PROPERTY(lrc::api::NewAccountModel* model READ getModel NOTIFY modelChanged)
    Q_PROPERTY(lrc::api::NewDeviceModel* deviceModel READ getDeviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(QString currentAccountId MEMBER currentAccountId_ NOTIFY currentAccountIdChanged)
    Q_PROPERTY(lrc::api::profile::Type currentAccountType MEMBER currentAccountType_ NOTIFY
                   currentAccountTypeChanged)
    Q_PROPERTY(int accountListSize MEMBER accountListSize_ NOTIFY accountListSizeChanged)

public:
    lrc::api::NewAccountModel* getModel();
    lrc::api::NewDeviceModel* getDeviceModel();

Q_SIGNALS:
    void modelChanged();
    void deviceModelChanged();
    void currentAccountIdChanged();
    void currentAccountTypeChanged();
    void accountListSizeChanged();

public:
    explicit AccountAdapter(AppSettingsManager* settingsManager,
                            LRCInstance* instance,
                            QObject* parent = nullptr);
    ~AccountAdapter() = default;

protected:
    void safeInit() override;

public:
    // Change to account corresponding to combox box index.
    Q_INVOKABLE void changeAccount(int row);

    // Create normal Jami account, SIP account and JAMS accounts.
    Q_INVOKABLE void createJamiAccount(QString registeredName,
                                       const QVariantMap& settings,
                                       bool isCreating);
    Q_INVOKABLE void createSIPAccount(const QVariantMap& settings);
    Q_INVOKABLE void createJAMSAccount(const QVariantMap& settings);

    // Delete current account
    Q_INVOKABLE void deleteCurrentAccount();

    // Setting related
    Q_INVOKABLE void passwordSetStatusMessageBox(bool success, QString title, QString infoToDisplay);

    // Conf property
    Q_INVOKABLE bool hasPassword();
    Q_INVOKABLE void setArchiveHasPassword(bool isHavePassword);
    Q_INVOKABLE bool exportToFile(const QString& accountId,
                                  const QString& path,
                                  const QString& password = {}) const;
    Q_INVOKABLE void setArchivePasswordAsync(const QString& accountID, const QString& password);

    // Lrc instances functions wrappers
    Q_INVOKABLE bool savePassword(const QString& accountId,
                                  const QString& oldPassword,
                                  const QString& newPassword);

    Q_INVOKABLE void startPreviewing(bool force = false);
    Q_INVOKABLE void stopPreviewing();
    Q_INVOKABLE bool hasVideoCall();
    Q_INVOKABLE bool isPreviewing();
    Q_INVOKABLE void setCurrAccDisplayName(const QString& text);
    Q_INVOKABLE lrc::api::profile::Type getCurrentAccountType();

    Q_INVOKABLE void setCurrAccAvatar(bool fromFile, const QString& source);

Q_SIGNALS:
    // Trigger other components to reconnect account related signals.
    void accountStatusChanged(QString accountId);
    void selectedContactAdded(QString convId);

    // Send report failure to QML to make it show the right UI state .
    void reportFailure();
    void accountAdded(QString accountId, bool showBackUp, int index);
    void contactUnbanned();

private Q_SLOTS:
    void onCurrentAccountChanged();

private:
    QString currentAccountId_ {};
    lrc::api::profile::Type currentAccountType_ {};
    int accountListSize_ {};

    // Make account signal connections.
    void connectAccount(const QString& accountId);

    // Make account signal connections.
    void setProperties(const QString& accountId);

    // Implement what to do when account creation fails.
    void connectFailure();

    QMetaObject::Connection accountStatusChangedConnection_;
    QMetaObject::Connection accountProfileUpdatedConnection_;
    QMetaObject::Connection contactAddedConnection_;
    QMetaObject::Connection addedToConferenceConnection_;
    QMetaObject::Connection bannedStatusChangedConnection_;
    QMetaObject::Connection registeredNameSavedConnection_;

    AppSettingsManager* settingsManager_;
};
Q_DECLARE_METATYPE(AccountAdapter*)
