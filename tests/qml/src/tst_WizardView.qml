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

import QtQuick
import QtTest

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1
import net.jami.Enums 1.1

import "qrc:/src/wizardview"
import "qrc:/src/commoncomponents"

WizardView {
    id: uut

    function clearSignalSpy() {
        spyAccountIsReady.clear()
        spyAccountIsRemoved.clear()
        spyAccountConfigFinalized.clear()
        spyReportFailure.clear()
        spyCloseWizardView.clear()

        spyBackButtonVisible.target = undefined
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

    SignalSpy {
        id: spyBackButtonVisible

        signalName: "visibleChanged"
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
            compare(CurrentAccount.alias, aliasText)

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
            compare(CurrentAccount.isRendezVous, true)

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
            compare(CurrentAccount.alias, aliasText)

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
            var createAccountButton = findChild(createSIPAccountPage, "createSIPAccountButton")

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
            compare(CurrentAccount.routeset, proxy)
            compare(CurrentAccount.username, userName)
            compare(CurrentAccount.hostname, serverName)
            compare(CurrentAccount.password, password)

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
            var connectBtn = findChild(importFromBackupPage, "importFromBackupPageConnectBtn")
            var errorLabel = findChild(importFromBackupPage, "errorLabel")
            var backButton = findChild(importFromBackupPage, "importFromBackupPageBackButton")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to importFromBackup page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.ImportFromBackup)

            spyBackButtonVisible.target = backButton

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
            spyBackButtonVisible.wait()
            verify(spyBackButtonVisible.count >= 2)
            spyBackButtonVisible.clear()

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

    TestCase {
        name: "Wizardview key navigation"
        when: windowShown

        function test_welcomePageKeyNavigation() {
            var welcomePage = findChild(uut, "welcomePage")

            var newAccountButton = findChild(welcomePage, "newAccountButton")
            var newRdvButton = findChild(welcomePage, "newRdvButton")
            var fromDeviceButton = findChild(welcomePage, "fromDeviceButton")
            var fromBackupButton = findChild(welcomePage, "fromBackupButton")
            var showAdvancedButton = findChild(welcomePage, "showAdvancedButton")
            var connectAccountManagerButton = findChild(welcomePage, "connectAccountManagerButton")
            var newSIPAccountButton = findChild(welcomePage, "newSIPAccountButton")
            var welcomePageBackButton = findChild(welcomePage, "welcomePageBackButton")

            welcomePageBackButton.visible = true

            // ShowAdvanced is false
            keyClick(Qt.Key_Tab)
            compare(newAccountButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(newRdvButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(fromDeviceButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(fromBackupButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(showAdvancedButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(welcomePageBackButton.focus, true)

            // Set showAdvanced to true
            keyClick(Qt.Key_Up)
            compare(showAdvancedButton.focus, true)

            keyClick(Qt.Key_Enter)
            compare(showAdvancedButton.showAdvanced, true)

            keyClick(Qt.Key_Tab)
            compare(connectAccountManagerButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(newSIPAccountButton.focus, true)

            // Use down button
            keyClick(Qt.Key_Down)
            compare(welcomePageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(newAccountButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(newRdvButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(fromDeviceButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(fromBackupButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(showAdvancedButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(connectAccountManagerButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(newSIPAccountButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(welcomePageBackButton.focus, true)

            // Use up button
            keyClick(Qt.Key_Up)
            compare(newSIPAccountButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(connectAccountManagerButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(showAdvancedButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(fromBackupButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(fromDeviceButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(newRdvButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(newAccountButton.focus, true)
        }

        function test_createAccountPageKeyNavigation() {
            uut.clearSignalSpy()

            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")

            var newAccountButton = findChild(welcomePage, "newAccountButton")

            var usernameEdit = findChild(createAccountPage, "usernameEdit")
            var chooseUsernameButton = findChild(createAccountPage,
                                                 "chooseUsernameButton")
            var nameRegistrationPageSkipButton = findChild(createAccountPage,
                                                           "nameRegistrationPageSkipButton")
            var passwordEdit = findChild(createAccountPage, "passwordEdit")
            var passwordSwitch = findChild(createAccountPage, "passwordSwitch")
            var passwordConfirmEdit = findChild(createAccountPage, "passwordConfirmEdit")
            var createAccountButton = findChild(createAccountPage,
                                                "createAccountButton")
            var createAccountPageBackButton = findChild(createAccountPage,
                                                        "createAccountPageBackButton")

            // To createAccountPage - nameRegistrationPage
            keyClick(Qt.Key_Tab)
            compare(newAccountButton.focus, true)

            keyClick(Qt.Key_Enter)
            compare(usernameEdit.focus, true)

            // No username
            keyClick(Qt.Key_Tab)
            compare(nameRegistrationPageSkipButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(usernameEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(nameRegistrationPageSkipButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(usernameEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(nameRegistrationPageSkipButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(usernameEdit.focus, true)

            // With username
            usernameEdit.nameRegistrationState =
                    UsernameLineEdit.NameRegistrationState.FREE

            keyClick(Qt.Key_Tab)
            compare(chooseUsernameButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(nameRegistrationPageSkipButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(usernameEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(chooseUsernameButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(nameRegistrationPageSkipButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(usernameEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(nameRegistrationPageSkipButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(chooseUsernameButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(usernameEdit.focus, true)

            // To createAccountPage - passwordSetupPage
            keyClick(Qt.Key_Enter)
            keyClick(Qt.Key_Tab)
            compare(passwordSwitch.focus, true)

            // No password
            keyClick(Qt.Key_Tab)
            compare(createAccountButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordSwitch.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordSwitch.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordSwitch.focus, true)

            // With password - no text entered
            keyClick(Qt.Key_Enter)
            compare(passwordEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordConfirmEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordSwitch.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordConfirmEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordSwitch.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordConfirmEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordSwitch.focus, true)

            // With password - with text entered
            passwordEdit.text = "test"
            passwordConfirmEdit.text = "test"

            keyClick(Qt.Key_Tab)
            compare(passwordEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordConfirmEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createAccountButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordSwitch.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordConfirmEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordSwitch.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(createAccountButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordConfirmEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordSwitch.focus, true)

            passwordEdit.text = ""
            passwordConfirmEdit.text = ""

            // Check lineEdit enter key press corrspond correctly
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Enter)
            keyClick(Qt.Key_Enter)
            compare(passwordConfirmEdit.focus, true)

            passwordEdit.text = "test"
            passwordConfirmEdit.text = "test"

            keyClick(Qt.Key_Enter)

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)

            // Go back to welcomePage
            WizardViewStepModel.nextStep()

            var showBackup = (WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateJamiAccount
                              || WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateRendezVous)
                              && !AppSettingsManager.getValue(Settings.NeverShowMeAgain)
            if (showBackup) {
                WizardViewStepModel.nextStep()
            }

            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }

        function test_importFromDevicePageKeyNavigation() {
            uut.clearSignalSpy()

            var welcomePage = findChild(uut, "welcomePage")
            var importFromDevicePage = findChild(uut, "importFromDevicePage")

            var fromDeviceButton = findChild(welcomePage, "fromDeviceButton")

            var pinFromDevice = findChild(importFromDevicePage, "pinFromDevice")
            var importFromDevicePageConnectBtn = findChild(importFromDevicePage,
                                                           "importFromDevicePageConnectBtn")
            var passwordFromDevice = findChild(importFromDevicePage, "passwordFromDevice")
            var importFromDevicePageBackButton = findChild(importFromDevicePage,
                                                           "importFromDevicePageBackButton")

            // To importFromDevicePage
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            compare(fromDeviceButton.focus, true)

            keyClick(Qt.Key_Enter)
            compare(pinFromDevice.focus, true)

            // No device pin
            keyClick(Qt.Key_Tab)
            compare(importFromDevicePageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Tab)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Down)
            compare(importFromDevicePageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Down)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            compare(importFromDevicePageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(pinFromDevice.focus, true)

            // With device pin
            pinFromDevice.text = "test"

            keyClick(Qt.Key_Tab)
            compare(importFromDevicePageConnectBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(importFromDevicePageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Tab)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Down)
            compare(importFromDevicePageConnectBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(importFromDevicePageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Down)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            compare(importFromDevicePageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(importFromDevicePageConnectBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(pinFromDevice.focus, true)

            // Account creation in process
            importFromDevicePageConnectBtn.spinnerTriggered = true

            keyClick(Qt.Key_Tab)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Tab)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Down)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            compare(pinFromDevice.focus, true)

            importFromDevicePageConnectBtn.spinnerTriggered = false

            // Check lineEdit enter key press corrspond correctly
            keyClick(Qt.Key_Enter)
            compare(pinFromDevice.focus, true)

            keyClick(Qt.Key_Up)
            keyClick(Qt.Key_Enter)
            compare(pinFromDevice.focus, true)

            pinFromDevice.text = "test"
            keyClick(Qt.Key_Enter)

            spyReportFailure.wait(15000)
            verify(spyReportFailure.count >= 1)

            // Go back to welcomePage
            keyClick(Qt.Key_Up)
            keyClick(Qt.Key_Up)
            keyClick(Qt.Key_Enter)
        }

        function test_importFromBackupPageKeyNavigation() {
            uut.clearSignalSpy()

            var welcomePage = findChild(uut, "welcomePage")
            var importFromBackupPage = findChild(uut, "importFromBackupPage")

            var fromBackupButton = findChild(welcomePage, "fromBackupButton")

            var passwordFromBackupEdit = findChild(importFromBackupPage, "passwordFromBackupEdit")
            var importFromBackupPageBackButton = findChild(importFromBackupPage,
                                                           "importFromBackupPageBackButton")
            var importFromBackupPageConnectBtn = findChild(importFromBackupPage,
                                                           "importFromBackupPageConnectBtn")
            var fileImportBtn = findChild(importFromBackupPage, "fileImportBtn")

            // To importFromBackupPage
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            compare(fromBackupButton.focus, true)

            keyClick(Qt.Key_Enter)
            compare(passwordFromBackupEdit.focus, true)

            // No filePath loaded
            keyClick(Qt.Key_Tab)
            compare(importFromBackupPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordFromBackupEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(importFromBackupPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordFromBackupEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(importFromBackupPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordFromBackupEdit.focus, true)

            // With filePath loaded
            importFromBackupPage.filePath = "test"

            keyClick(Qt.Key_Tab)
            compare(importFromBackupPageConnectBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(importFromBackupPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordFromBackupEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(importFromBackupPageConnectBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(importFromBackupPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordFromBackupEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(importFromBackupPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(importFromBackupPageConnectBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordFromBackupEdit.focus, true)

            // Account creation in process
            importFromBackupPageConnectBtn.spinnerTriggered = true

            keyClick(Qt.Key_Tab)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(passwordFromBackupEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(passwordFromBackupEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(fileImportBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(passwordFromBackupEdit.focus, true)

            importFromBackupPageConnectBtn.spinnerTriggered = false

            // Check lineEdit enter key press corrspond correctly
            var fileName = "gz_test.gz"
            var wrongPassword = "ccc"
            importFromBackupPage.filePath = UtilsAdapter.toFileAbsolutepath(
                        "tests/qml/src/resources/gz_test.gz") + "/" + fileName
            passwordFromBackupEdit.text = wrongPassword

            keyClick(Qt.Key_Enter)

            spyReportFailure.wait(15000)
            verify(spyReportFailure.count >= 1)

            // Go back to welcomePage
            keyClick(Qt.Key_Up)
            keyClick(Qt.Key_Up)
            keyClick(Qt.Key_Enter)
        }

        function test_createSIPAccountPageKeyNavigation() {
            uut.clearSignalSpy()

            var welcomePage = findChild(uut, "welcomePage")
            var createSIPAccountPage = findChild(uut, "createSIPAccountPage")

            var showAdvancedButton = findChild(welcomePage, "showAdvancedButton")

            var newSIPAccountButton = findChild(welcomePage, "newSIPAccountButton")

            var sipServernameEdit = findChild(createSIPAccountPage, "sipServernameEdit")
            var sipProxyEdit = findChild(createSIPAccountPage, "sipProxyEdit")
            var sipUsernameEdit = findChild(createSIPAccountPage, "sipUsernameEdit")
            var sipPasswordEdit = findChild(createSIPAccountPage, "sipPasswordEdit")
            var createSIPAccountButton = findChild(createSIPAccountPage, "createSIPAccountButton")
            var createSIPAccountPageBackButton = findChild(createSIPAccountPage,
                                                           "createSIPAccountPageBackButton")

            // To connectToAccountManagerPage
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Up)
            keyClick(Qt.Key_Enter)
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            compare(newSIPAccountButton.focus, true)

            keyClick(Qt.Key_Enter)
            compare(sipServernameEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(sipProxyEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(sipUsernameEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(sipPasswordEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createSIPAccountButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(createSIPAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(sipServernameEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(sipProxyEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(sipUsernameEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(sipPasswordEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(createSIPAccountButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(createSIPAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(sipServernameEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(createSIPAccountPageBackButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(createSIPAccountButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(sipPasswordEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(sipUsernameEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(sipProxyEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(sipServernameEdit.focus, true)

            // Check lineEdit enter key press corrspond correctly
            keyClick(Qt.Key_Enter)
            keyClick(Qt.Key_Enter)
            keyClick(Qt.Key_Enter)
            keyClick(Qt.Key_Enter)

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)

            spyAccountStatusChanged.wait()
            verify(spyAccountStatusChanged.count >= 1)

            WizardViewStepModel.nextStep()

            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)

            // Hide advanced options
            showAdvancedButton.clicked()
        }

        function test_profilePageKeyNavigation() {
            uut.clearSignalSpy()

            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")
            var profilePage = findChild(uut, "profilePage")
            var backupKeysPage = findChild(uut, "backupKeysPage")

            var createAccountButton = findChild(createAccountPage, "createAccountButton")

            var aliasEdit = findChild(profilePage, "aliasEdit")
            var saveProfileBtn = findChild(profilePage, "saveProfileBtn")
            var setAvatarWidget = findChild(profilePage, "setAvatarWidget")
            var skipProfileSavingButton = findChild(profilePage, "skipProfileSavingButton")

            var photoboothImportFromFileDialog = findChild(setAvatarWidget,
                                                           "photoboothImportFromFileDialog")
            var takePhotoButton = findChild(setAvatarWidget, "takePhotoButton")
            var photoboothViewClearButton = findChild(setAvatarWidget,
                                                      "photoboothViewClearButton")
            var photoboothViewImportButton = findChild(setAvatarWidget,
                                                       "photoboothViewImportButton")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateJamiAccount)

            // Go to set up password page
            WizardViewStepModel.nextStep()
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

            // Navigation test
            keyClick(Qt.Key_Tab)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(photoboothViewImportButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(photoboothViewImportButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(photoboothViewImportButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(aliasEdit.focus, true)

            // Set up photo from fake JamiFileDialog imported file
            spyAccountStatusChanged.clear()
            photoboothViewImportButton.focusAfterFileDialogClosed = true
            photoboothImportFromFileDialog.file = UtilsAdapter.toFileAbsolutepath(
                        "tests/qml/src/resources/png_test.png") + "/" + "png_test.png"
            photoboothImportFromFileDialog.accepted()

            spyAccountStatusChanged.wait()
            verify(spyAccountStatusChanged.count >= 1)
            compare(photoboothViewImportButton.focus, true)
            compare(photoboothViewClearButton.visible, true)

            keyClick(Qt.Key_Tab)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(photoboothViewClearButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(photoboothViewImportButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(photoboothViewClearButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(photoboothViewImportButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(photoboothViewClearButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(photoboothViewImportButton.focus, true)

            // Clear photo
            keyClick(Qt.Key_Up)
            compare(photoboothViewClearButton.focus, true)
            keyClick(Qt.Key_Enter)
            compare(takePhotoButton.focus, true)

            // Taking photo focus test
            setAvatarWidget.isPreviewing = true

            keyClick(Qt.Key_Tab)
            compare(photoboothViewClearButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Tab)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Tab)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Tab)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(photoboothViewClearButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Down)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Down)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Down)
            compare(takePhotoButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(skipProfileSavingButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(saveProfileBtn.focus, true)

            keyClick(Qt.Key_Up)
            compare(aliasEdit.focus, true)

            keyClick(Qt.Key_Up)
            compare(photoboothViewClearButton.focus, true)

            keyClick(Qt.Key_Up)
            compare(takePhotoButton.focus, true)

            setAvatarWidget.isPreviewing = false

            // Check lineEdit enter key press corrspond correctly
            var aliasName = "test"
            aliasEdit.text = aliasName
            spyAccountStatusChanged.clear()

            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Tab)
            keyClick(Qt.Key_Enter)

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

            spyAccountStatusChanged.wait()
            verify(spyAccountStatusChanged.count >= 1)

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            // Check alias text
            compare(CurrentAccount.alias, aliasName)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }

        function test_backupKeysPageNavigation() {
            uut.clearSignalSpy()

            var controlPanelStackView = findChild(uut, "controlPanelStackView")

            var welcomePage = findChild(uut, "welcomePage")
            var createAccountPage = findChild(uut, "createAccountPage")
            var profilePage = findChild(uut, "profilePage")
            var backupKeysPage = findChild(uut, "backupKeysPage")

            var createAccountButton = findChild(createAccountPage, "createAccountButton")
            var skipProfileSavingButton = findChild(profilePage, "skipProfileSavingButton")

            var neverShowMeAgainSwitch = findChild(backupKeysPage, "neverShowMeAgainSwitch")
            var backupKeyPageBackupBtn = findChild(backupKeysPage, "backupKeyPageBackupBtn")
            var backupKeyPageSkipBackupBtn = findChild(backupKeysPage, "backupKeyPageSkipBackupBtn")

            // WelcomePage initially
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    welcomePage)

            // Go to createAccount page
            WizardViewStepModel.startAccountCreationFlow(
                        WizardViewStepModel.AccountCreationOption.CreateJamiAccount)

            // Go to set up password page
            WizardViewStepModel.nextStep()
            createAccountButton.clicked()

            // Wait until the account creation is finished
            spyAccountIsReady.wait()
            compare(spyAccountIsReady.count, 1)

            // Now we are in profile page
            compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                    profilePage)
            spyAccountConfigFinalized.wait()
            compare(spyAccountConfigFinalized.count, 1)

            skipProfileSavingButton.clicked()

            var showBackup = (WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateJamiAccount
                              || WizardViewStepModel.accountCreationOption ===
                              WizardViewStepModel.AccountCreationOption.CreateRendezVous)
                              && !AppSettingsManager.getValue(Settings.NeverShowMeAgain)
            if (showBackup) {
                compare(controlPanelStackView.children[controlPanelStackView.currentIndex],
                        backupKeysPage)

                // Navigation test
                compare(neverShowMeAgainSwitch.focus, true)

                keyClick(Qt.Key_Tab)
                compare(backupKeyPageBackupBtn.focus, true)

                keyClick(Qt.Key_Tab)
                compare(backupKeyPageSkipBackupBtn.focus, true)

                keyClick(Qt.Key_Tab)
                compare(neverShowMeAgainSwitch.focus, true)

                keyClick(Qt.Key_Down)
                compare(backupKeyPageBackupBtn.focus, true)

                keyClick(Qt.Key_Down)
                compare(backupKeyPageSkipBackupBtn.focus, true)

                keyClick(Qt.Key_Down)
                compare(neverShowMeAgainSwitch.focus, true)

                keyClick(Qt.Key_Up)
                compare(backupKeyPageSkipBackupBtn.focus, true)

                keyClick(Qt.Key_Up)
                compare(backupKeyPageBackupBtn.focus, true)

                keyClick(Qt.Key_Up)
                compare(neverShowMeAgainSwitch.focus, true)

                WizardViewStepModel.nextStep()
            }

            spyCloseWizardView.wait()
            compare(spyCloseWizardView.count, 1)

            AccountAdapter.deleteCurrentAccount()

            // Wait until the account removal is finished
            spyAccountIsRemoved.wait()
            compare(spyAccountIsRemoved.count, 1)
        }
    }
}
