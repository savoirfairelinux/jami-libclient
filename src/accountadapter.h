/*
 * Copyright (C) 2021 by Savoir-faire Linux
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

#include "accountlistmodel.h"
#include "lrcinstance.h"
#include "utils.h"

#include <QSettings>
#include <QString>

class AppSettingsManager;

class AccountAdapter final : public QmlAdapterBase
{
    Q_OBJECT

    Q_PROPERTY(lrc::api::NewAccountModel* model READ getModel NOTIFY modelChanged)

public:
    lrc::api::NewAccountModel* getModel();

Q_SIGNALS:
    void modelChanged();

public:
    explicit AccountAdapter(AppSettingsManager* settingsManager,
                            LRCInstance* instance,
                            QObject* parent = nullptr);
    ~AccountAdapter() = default;

    void safeInit() override {}

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
    Q_INVOKABLE bool hasVideoCall();
    Q_INVOKABLE void setCurrAccDisplayName(const QString& text);
    Q_INVOKABLE void setCurrentAccountAvatarFile(const QString& source);
    Q_INVOKABLE void setCurrentAccountAvatarBase64(const QString& source = {});
    Q_INVOKABLE void setDefaultModerator(const QString& accountId,
                                         const QString& peerURI,
                                         const bool& state);
    Q_INVOKABLE QStringList getDefaultModerators(const QString& accountId);

Q_SIGNALS:
    // Trigger other components to reconnect account related signals.
    void accountStatusChanged(QString accountId);

    // Send report failure to QML to make it show the right UI state .
    void reportFailure();
    void accountCreationFailed();
    void accountAdded(QString accountId, int index);
    void accountRemoved();
    void accountConfigFinalized();

private:
    // Implement what to do when account creation fails.
    void connectFailure();

    QMetaObject::Connection registeredNameSavedConnection_;

    AppSettingsManager* settingsManager_;

    QScopedPointer<AccountListModel> accSrcModel_;
    QScopedPointer<CurrentAccountFilterModel> accModel_;
};
Q_DECLARE_METATYPE(AccountAdapter*)
