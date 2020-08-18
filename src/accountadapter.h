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

class AccountAdapter : public QmlAdapterBase
{
    Q_OBJECT

public:
    explicit AccountAdapter(QObject *parent = 0);
    ~AccountAdapter();
    //Singleton
    static AccountAdapter &instance();

    /*
     * Change to account corresponding to combox box index.
     */
    Q_INVOKABLE void accountChanged(int index);
    /*
     * Create normal Jami account, SIP account and JAMS accounts.
     */
    Q_INVOKABLE void createJamiAccount(QString registeredName,
                                       const QVariantMap &settings,
                                       QString photoBoothImgBase64,
                                       bool isCreating);
    Q_INVOKABLE void createSIPAccount(const QVariantMap &settings, QString photoBoothImgBase64);
    Q_INVOKABLE void createJAMSAccount(const QVariantMap &settings);
    /*
     * Delete current account
     */
    Q_INVOKABLE void deleteCurrentAccount();
    /*
     * Setting related
     */
    Q_INVOKABLE void settingsNeverShowAgain(bool checked);
    Q_INVOKABLE void passwordSetStatusMessageBox(bool success, QString title, QString infoToDisplay);
    /*
     * conf property
     */
    Q_INVOKABLE bool hasPassword();
    Q_INVOKABLE void setArchiveHasPassword(bool isHavePassword);
    Q_INVOKABLE bool exportToFile(const QString &accountId,
                                  const QString &path,
                                  const QString &password = {}) const;
    Q_INVOKABLE void setArchivePasswordAsync(const QString &accountID, const QString &password);
    /*
     * lrc instances functions wrappers
     */
    Q_INVOKABLE bool savePassword(QString accountId, QString oldPassword, QString newPassword);
    Q_INVOKABLE void startAudioMeter(bool async);
    Q_INVOKABLE void stopAudioMeter(bool async);
    Q_INVOKABLE void startPreviewing(bool force = false, bool async = true);
    Q_INVOKABLE void stopPreviewing(bool async = true);
    Q_INVOKABLE bool hasVideoCall();
    Q_INVOKABLE bool isPreviewing();
    Q_INVOKABLE void setCurrAccDisplayName(QString text);
    Q_INVOKABLE void setSelectedAccountId(QString accountId = {});
    Q_INVOKABLE void setSelectedConvId(QString accountId = {});

signals:

    /*
     * Trigger other components to reconnect account related signals.
     */
    void accountSignalsReconnect(const QString &accountId);
    void accountStatusChanged();
    void updateConversationForAddedContact();
    /*
     * send report failure to QML to make it show the right UI state .
     */
    void reportFailure();
    void accountAdded(bool showBackUp, int index);

private:
    void initQmlObject() override final;
    void setSelectedAccount(const QString &accountId);
    void backToWelcomePage();
    void deselectConversation();

    /*
     * Make account signal connections.
     */
    void connectAccount(const QString &accountId);
    /*
     * Implement what to do when creat accout fails.
     */
    void connectFailure();

    QMetaObject::Connection accountStatusChangedConnection_;
    QMetaObject::Connection contactAddedConnection_;
    QMetaObject::Connection addedToConferenceConnection_;
    QMetaObject::Connection accountProfileChangedConnection_;
};
Q_DECLARE_METATYPE(AccountAdapter *)
