/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.14
import QtTest 1.2

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0
import net.jami.Enums 1.0

import "qrc:/src/wizardview"

WizardView {
    id: uut

    function clearSignalSpy() {
        spyAccountIsReady.clear()
        spyAccountIsRemoved.clear()
        spyAccountConfigFinalized.clear()
        spyReportFailure.clear()
        spyCloseWizardView.clear()
    }

    SignalSpy {
        id: spyAccountIsReady

        target: WizardViewStepModel
        signalName: "accountIsReady"
    }

    SignalSpy {
        id: spyAccountIsRemoved

        target: AccountAdapter
        signalName: "accountRemoved"
    }

    SignalSpy {
        id: spyAccountStatusChanged

        target: AccountAdapter
        signalName: "accountStatusChanged"
    }

    SignalSpy {
        id: spyAccountConfigFinalized

        target: AccountAdapter
        signalName: "accountConfigFinalized"
    }

    SignalSpy {
        id: spyReportFailure

        target: AccountAdapter
        signalName: "reportFailure"
    }

    SignalSpy {
        id: spyCloseWizardView

        target: WizardViewStepModel
        signalName: "closeWizardView"
    }

    TestCase {
        name: "WelcomePage to different account creation page and return back"
        when: windowShown

        function test_welcomePageStepInStepOut() {
            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")
            var importFromDevicePage = findChild(uut, "importFromDevicePage")
            var importFromBackupPage = findChild(uut, "importFromBackupPage")
            var connectToAccountManagerPage = findChild(uut, "connectToAccountManagerPage")
            var createSIPAccountPage = findChild(uut, "createSIPAccountPage")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateJamiAccount)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    createAccountPage)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to CreateRendezVous page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateRendezVous)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    createAccountPage)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to CreateRendezVous page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.ImportFromDevice)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    importFromDevicePage)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to ImportFromBackup page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.ImportFromBackup)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    importFromBackupPage)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to ConnectToAccountManager page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.ConnectToAccountManager)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    connectToAccountManagerPage)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to CreateSipAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateSipAccount)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    createSIPAccountPage)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)
        }

        function test_createAccountPageStepInStepOut() {
            var controlPanelStackView = findChild(uut, "controlPanelStackView")
            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")

            var createAccountStack = findChild(createAccountPage, "createAccountStack")
            var passwordSetupPage = findChild(createAccountPage, "passwordSetupPage")
            var nameRegistrationPage = findChild(createAccountPage, "nameRegistrationPage")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateJamiAccount)
            compare(createAccountPage.isRendezVous, false)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    createAccountPage)
            compare(createAccountStack.currentIndex, nameRegistrationPage.stackIndex)

            // Go to passwordSetup page
            WizardViewStepModel.nextStep()
            compare(createAccountStack.currentIndex, passwordSetupPage.stackIndex)

            // Back
            WizardViewStepModel.previousStep()
            compare(createAccountStack.currentIndex, nameRegistrationPage.stackIndex)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to CreateRendezVous page (createAccount)
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateRendezVous)
            compare(createAccountPage.isRendezVous, true)
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    createAccountPage)
            compare(createAccountStack.currentIndex, nameRegistrationPage.stackIndex)

            // Go to passwordSetup page
            WizardViewStepModel.nextStep()
            compare(createAccountStack.currentIndex, passwordSetupPage.stackIndex)

            // Back
            WizardViewStepModel.previousStep()
            compare(createAccountStack.currentIndex, nameRegistrationPage.stackIndex)
            WizardViewStepModel.previousStep()
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)
        }
    }

    TestCase {
        name: "Create Jami account ui flow (no registered name)"
        when: windowShown

        function test_createJamiAccountUiFlow() {
            uut.clearSignalSpy()

            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")
            var profilePage = findChild(uut, "profilePage")
            var backupKeysPage = findChild(uut, "backupKeysPage")

            var usernameEdit = findChild(createAccountPage, "usernameEdit")
            var createAccountStack = findChild(createAccountPage, "createAccountStack")
            var passwordSwitch = findChild(createAccountPage, "passwordSwitch")
            var passwordEdit = findChild(createAccountPage, "passwordEdit")
            var passwordConfirmEdit = findChild(createAccountPage, "passwordConfirmEdit")
            var createAccountButton = findChild(createAccountPage, "createAccountButton")

            var aliasEdit = findChild(profilePage, "aliasEdit")
            var saveProfileBtn = findChild(profilePage, "saveProfileBtn")

            var password  = "test110"
            var aliasText = "test101"

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateJamiAccount)
            compare(createAccountStack.currentIndex, 0)
            compare(usernameEdit.focus, true)

            // Go to set up password page
            WizardViewStepModel.nextStep()
            compare(createAccountStack.currentIndex, 1)
            passwordSwitch.checked = true
            compare(passwordEdit.focus, true)
            passwordEdit.text = password
            passwordConfirmEdit.text = password
            createAccountButton.clicked()

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)

            // Now we are in profile page
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    profilePage)
            compare(aliasEdit.focus, true)

            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)

            aliasEdit.text = aliasText
            saveProfileBtn.clicked()

            var showBackup = (WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateJamiAccount
                              || WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateRendezVous)
                              && !AppSettingsManager.getValue(Settings.NeverShowMeAgain)
            if (showBackup) {
                compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                        backupKeysPage)
                WizardViewStepModel.nextStep()
            }

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            // Check alias text
            compare(SettingsAdapter.getCurrentAccount_Profile_Info_Alias(), aliasText)

            spyAccountStatusChanged.clear()

            // Check if password is set
            compare(AccountAdapter.savePassword(LRCInstance.currentAccountId, password, "test"),
                    true)

            // Wait until the account status change is finished
            spyAccountStatusChanged.wait()
            verify(spyAccountStatusChanged.count >= 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }

        function test_createRendezVousAccountUiFlow() {
            uut.clearSignalSpy()

            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")
            var profilePage = findChild(uut, "profilePage")
            var backupKeysPage = findChild(uut, "backupKeysPage")

            var usernameEdit = findChild(createAccountPage, "usernameEdit")
            var createAccountStack = findChild(createAccountPage, "createAccountStack")
            var passwordSwitch = findChild(createAccountPage, "passwordSwitch")
            var passwordEdit = findChild(createAccountPage, "passwordEdit")
            var passwordConfirmEdit = findChild(createAccountPage, "passwordConfirmEdit")
            var createAccountButton = findChild(createAccountPage, "createAccountButton")

            var aliasEdit = findChild(profilePage, "aliasEdit")
            var saveProfileBtn = findChild(profilePage, "saveProfileBtn")

            var password  = "test110"
            var aliasText = "test101"

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createRendezVous page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateRendezVous)
            compare(createAccountStack.currentIndex, 0)
            compare(usernameEdit.focus, true)

            // Go to set up password page
            WizardViewStepModel.nextStep()
            compare(createAccountStack.currentIndex, 1)
            passwordSwitch.checked = true
            compare(passwordEdit.focus, true)
            passwordEdit.text = password
            passwordConfirmEdit.text = password
            createAccountButton.clicked()

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)

            // Now we are in profile page
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    profilePage)
            compare(aliasEdit.focus, true)

            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)

            // Check if it is a RendezVous acc
            compare(SettingsAdapter.getAccountConfig_RendezVous(), true)

            aliasEdit.text = aliasText
            saveProfileBtn.clicked()

            var showBackup = (WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateJamiAccount
                              || WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateRendezVous)
                              && !AppSettingsManager.getValue(Settings.NeverShowMeAgain)
            if (showBackup) {
                compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                        backupKeysPage)
                WizardViewStepModel.nextStep()
            }

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            // Check alias text
            compare(SettingsAdapter.getCurrentAccount_Profile_Info_Alias(), aliasText)

            spyAccountStatusChanged.clear()

            // Check if password is set
            compare(AccountAdapter.savePassword(LRCInstance.currentAccountId, password, "test"),
                    true)

            // Wait until the account status change is finished
            spyAccountStatusChanged.wait()
            verify(spyAccountStatusChanged.count >= 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }
    }

    TestCase {
        name: "Create Sip account ui flow"
        when: windowShown

        function test_createSipAccountUiFlow() {
            uut.clearSignalSpy()

            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var createSIPAccountPage = findChild(uut, "createSIPAccountPage")
            var profilePage = findChild(uut, "profilePage")

            var sipUsernameEdit = findChild(createSIPAccountPage, "sipUsernameEdit")
            var sipPasswordEdit = findChild(createSIPAccountPage, "sipPasswordEdit")
            var sipServernameEdit = findChild(createSIPAccountPage, "sipServernameEdit")
            var sipProxyEdit = findChild(createSIPAccountPage, "sipProxyEdit")
            var createAccountButton = findChild(createSIPAccountPage, "createAccountButton")

            var saveProfileBtn = findChild(profilePage, "saveProfileBtn")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createSipAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateSipAccount)
            compare(sipServernameEdit.focus, true)

            // Set up paras
            var userName = "testUserName"
            var serverName = "testServerName"
            var password = "testPassword"
            var proxy = "testProxy"

            sipUsernameEdit.text = userName
            sipPasswordEdit.text = password
            sipServernameEdit.text = serverName
            sipProxyEdit.text = proxy

            createAccountButton.clicked()

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)

            // Now we are in profile page
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    profilePage)

            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)

            // Check if paras match with setup
            compare(SettingsAdapter.getAccountConfig_RouteSet(), proxy)
            compare(SettingsAdapter.getAccountConfig_Username(), userName)
            compare(SettingsAdapter.getAccountConfig_Hostname(), serverName)
            compare(SettingsAdapter.getAccountConfig_Password(), password)

            WizardViewStepModel.nextStep()

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }
    }

    TestCase {
        name: "Create Jami account from backup ui flow"
        when: windowShown

        function test_createJamiAccountFromBackupUiFlow() {
            uut.clearSignalSpy()

            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var importFromBackupPage = findChild(uut, "importFromBackupPage")
            var profilePage = findChild(uut, "profilePage")

            var passwordFromBackupEdit = findChild(importFromBackupPage, "passwordFromBackupEdit")
            var connectBtn = findChild(importFromBackupPage, "connectBtn")
            var errorLabel = findChild(importFromBackupPage, "errorLabel")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to importFromBackup page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.ImportFromBackup)
            compare(passwordFromBackupEdit.focus, true)

            var fileName = "gz_test.gz"
            var password = "qqq"
            importFromBackupPage.filePath = UtilsAdapter.toFileAbsolutepath(
                        "tests/qml/src/resources/gz_test.gz") + "/" + fileName

            compare(connectBtn.enabled, true)

            // Create without password
            connectBtn.clicked()
            spyReportFailure.wait()
            verify(spyReportFailure.count >= 1)

            compare(importFromBackupPage.errorText, JamiStrings.errorCreateAccount)
            compare(errorLabel.visible, true)

            // Recreate with password
            passwordFromBackupEdit.text = password
            connectBtn.clicked()

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)
            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)
            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }
    }
}
