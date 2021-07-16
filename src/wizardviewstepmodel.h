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

#include <QObject>
#include <QVariant>
#include <QMap>

#include "qtutils.h"

class AccountAdapter;
class LRCInstance;
class AppSettingsManager;

class WizardViewStepModel : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

public:
    enum class MainSteps {
        Initial,          // Initial welcome step.
        AccountCreation,  // General account creation step.
        NameRegistration, // Name registration step : CreateJamiAccount, CreateRendezVous
        SetPassword,      // Password set up step: CreateJamiAccount, CreateRendezVous
        Profile,          // Profile set up.
        BackupKeys        // Backup set up.
    };
    Q_ENUM(MainSteps)

    enum class AccountCreationOption {
        None,
        CreateJamiAccount,       // Jami account creation.
        CreateRendezVous,        // Jami rendezvous account creation.
        ImportFromDevice,        // Jami account creation from device.
        ImportFromBackup,        // Jami account creation from backup.
        ConnectToAccountManager, // Account manager creation.
        CreateSipAccount         // SIP account creation.
    };
    Q_ENUM(AccountCreationOption)

    QML_PROPERTY(MainSteps, mainStep)
    QML_PROPERTY(AccountCreationOption, accountCreationOption)

    QML_PROPERTY(QVariantMap, accountCreationInfo)

public:
    explicit WizardViewStepModel(LRCInstance* lrcInstance,
                                 AccountAdapter* accountAdapter,
                                 AppSettingsManager* appSettingsManager,
                                 QObject* parent = nullptr);

    Q_INVOKABLE void startAccountCreationFlow(AccountCreationOption accountCreationOption);
    Q_INVOKABLE void nextStep();
    Q_INVOKABLE void previousStep();

Q_SIGNALS:
    void accountIsReady(QString accountId);
    void closeWizardView();

private:
    void reset();

    LRCInstance* lrcInstance_;
    AccountAdapter* accountAdapter_;
    AppSettingsManager* appSettingsManager_;
};
