/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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
import QtQuick.Controls 1.4 as CT
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "../commoncomponents"
import "../constant"
import "components"

Rectangle {
    id: wizardViewWindow

    enum Mode {
        CREATE,
        IMPORT,
        MIGRATE,
        CREATESIP,
        CONNECTMANAGER
    }

    enum NameRegistrationState {
        BLANK,
        INVALID,
        TAKEN,
        FREE,
        SEARCHING
    }

    property int textFontSize: 9
    property int wizardMode: WizardView.CREATE
    property int addedAccountIndex: -1
    property bool showBackUp: false
    property bool showProfile: false
    property bool showBottom: false
    property string fileToImport: ""
    property string registeredName: ""

    property var inputParaObject: ({})

    /*
     * signal to redirect the page to main view
     */
    signal needToShowMainViewWindow(int accountIndex)
    signal wizardViewIsClosed

    visible: true
    anchors.fill: parent


    Component.onCompleted: {
        changePageQML(controlPanelStackView.welcomePageStackId)
    }

    Connections{
        target: ClientWrapper.accountAdaptor

        function onAccountAdded(showBackUp, index) {
            addedAccountIndex = index
            ClientWrapper.accountAdaptor.accountChanged(index)
            if (showProfile) {
                changePageQML(controlPanelStackView.profilePageId)
                profilePage.readyToSaveDetails = true
            } else if (controlPanelStackView.currentIndex == controlPanelStackView.profilePageId) {
                ClientWrapper.lrcInstance.accountListChanged()
                profilePage.readyToSaveDetails = true
            } else if (showBackUp) {
                changePageQML(controlPanelStackView.backupKeysPageId)
            } else {
                changePageQML(controlPanelStackView.welcomePageStackId)
                needToShowMainViewWindow(addedAccountIndex)
                ClientWrapper.lrcInstance.accountListChanged()
            }
        }

        // reportFailure
        function onReportFailure() {
            if (controlPanelStackView.currentIndex == controlPanelStackView.importFromDevicePageId) {
                importFromDevicePage.errorText = qsTr("Error when creating your account. Check your credentials")
            } else if (controlPanelStackView.currentIndex == controlPanelStackView.importFromBackupPageId) {
                importFromBackupPage.errorText = qsTr("Error when creating your account. Check your credentials")
            } else if (controlPanelStackView.currentIndex == controlPanelStackView.connectToAccountManagerPageId) {
                connectToAccountManagerPage.errorText = qsTr("Error when creating your account. Check your credentials")
            }
        }
    }

    Connections {
        id: registeredNameFoundConnection
        target: ClientWrapper.nameDirectory
        enabled: false

        function onRegisteredNameFound(status, address, name) {
            slotRegisteredNameFound(status, address, name)
        }
    }

    function slotRegisteredNameFound(status, address, name) {
        if (name.length != 0 && name.length < 3) {
            createAccountPage.nameRegistrationUIState = WizardView.INVALID
        } else if (registeredName === name) {
            switch (status) {
            case NameDirectory.LookupStatus.NOT_FOUND:
            case NameDirectory.LookupStatus.ERROR:
                createAccountPage.nameRegistrationUIState = WizardView.FREE
                break
            case NameDirectory.LookupStatus.INVALID_NAME:
            case NameDirectory.LookupStatus.INVALID:
                createAccountPage.nameRegistrationUIState = WizardView.INVALID
                break
            case NameDirectory.LookupStatus.SUCCESS:
                createAccountPage.nameRegistrationUIState = WizardView.TAKEN
                break
            }
        }
    }

    function changePageQML(pageIndex) {
        controlPanelStackView.currentIndex = pageIndex
        if (pageIndex == controlPanelStackView.welcomePageStackId) {
            fileToImport = ""
            registeredNameFoundConnection.enabled = true
            createAccountPage.nameRegistrationUIState = WizardView.BLANK
        } else if (pageIndex == controlPanelStackView.createAccountPageId) {
            createAccountPage.initializeOnShowUp()
            // connection between register name found and its slot
            registeredNameFoundConnection.enabled = true
        } else if (pageIndex == controlPanelStackView.createSIPAccountPageId) {
            createSIPAccountPage.initializeOnShowUp()
            btnNext.enabled = true
            // start photo booth
            createSIPAccountPage.startBooth()
        } else if (pageIndex == controlPanelStackView.importFromDevicePageId) {
            importFromDevicePage.initializeOnShowUp()
        } else if (pageIndex == controlPanelStackView.spinnerPageId) {
            createAccountPage.nameRegistrationUIState = WizardView.BLANK
            createAccountPage.isToSetPassword_checkState_choosePasswordCheckBox = false
        } else if (pageIndex == controlPanelStackView.connectToAccountManagerPageId) {
            connectToAccountManagerPage.initializeOnShowUp()
            btnNext.enabled = false
        } else if (pageIndex == controlPanelStackView.importFromBackupPageId) {
            importFromBackupPage.clearAllTextFields()
            fileToImport = ""
            btnNext.enabled = false
        } else if (pageIndex == controlPanelStackView.profilePageId) {
            profilePage.initializeOnShowUp()
            profilePage.showBottom = showBottom
        }
    }

    PasswordDialog {
        id: passwordDialog

        anchors.centerIn: parent.Center
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        visible: false
        purpose: PasswordDialog.ExportAccount

        onDoneSignal: {
            if (currentPurpose === passwordDialog.ExportAccount) {
                var success = (code === successCode)

                var title = success ? qsTr("Success") : qsTr("Error")
                var info = success ? qsTr("Export Successful") : qsTr(
                                         "Export Failed")

                ClientWrapper.accountAdaptor.passwordSetStatusMessageBox(success,
                                                         title, info)
                if (success) {
                    console.log("Account Export Succeed")
                    needToShowMainViewWindow(addedAccountIndex)
                    ClientWrapper.lrcInstance.accountListChanged()
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: forceActiveFocus()
    }

    ScrollView {
        id: frame
        clip: true
        anchors.fill: parent

        StackLayout {
            id: controlPanelStackView
            currentIndex: welcomePageStackId
            height: wizardView.height
            width: wizardView.width

            property int welcomePageStackId: 0
            property int createAccountPageId: 1
            property int createSIPAccountPageId: 2
            property int importFromBackupPageId: 3
            property int backupKeysPageId: 4
            property int importFromDevicePageId: 5
            property int connectToAccountManagerPageId: 6
            property int spinnerPageId: 7
            property int profilePageId: 8

            WelcomePageLayout {
                // welcome page, index 0
                id: welcomePage

                onWelcomePageRedirectPage: {
                    changePageQML(toPageIndex)
                }

                onLeavePage: {
                    wizardViewIsClosed()
                }
            }

            CreateAccountPage {
                // create account page, index 1
                id: createAccountPage

                onCreateAccount: {
                    inputParaObject = {}
                    inputParaObject["password"] = text_passwordEditAlias
                    ClientWrapper.accountAdaptor.createJamiAccount(
                        createAccountPage.text_usernameEditAlias,
                        inputParaObject,
                        createAccountPage.boothImgBase64,
                        true)
                    showBackUp = true
                    showBottom = true
                    changePageQML(controlPanelStackView.profilePageId)
                }

                onText_usernameEditAliasChanged: {
                    lookupTimer.restart()
                }

                onLeavePage: {
                    changePageQML(controlPanelStackView.welcomePageStackId)
                }

                Timer {
                    id: lookupTimer

                    repeat: false
                    triggeredOnStart: false
                    interval: 200

                    onTriggered: {
                        registeredName = createAccountPage.text_usernameEditAlias
                        if (registeredName.length !== 0) {
                            createAccountPage.nameRegistrationUIState = WizardView.SEARCHING
                            ClientWrapper.nameDirectory.lookupName("", registeredName)
                        } else {
                            createAccountPage.nameRegistrationUIState = WizardView.BLANK
                        }
                    }
                }
            }

            CreateSIPAccountPage {
                // create SIP account page, index 2
                id: createSIPAccountPage

                onLeavePage: {
                    changePageQML(controlPanelStackView.welcomePageStackId)
                }

                onCreateAccount: {
                    inputParaObject = {}
                    inputParaObject["hostname"] = createSIPAccountPage.text_sipServernameEditAlias
                    inputParaObject["username"] = createSIPAccountPage.text_sipUsernameEditAlias
                    inputParaObject["password"] = createSIPAccountPage.text_sipPasswordEditAlias
                    inputParaObject["proxy"] = createSIPAccountPage.text_sipProxyEditAlias
                    createSIPAccountPage.clearAllTextFields()

                    ClientWrapper.accountAdaptor.createSIPAccount(inputParaObject, "")
                    showBackUp = false
                    showBottom = false
                    changePageQML(controlPanelStackView.profilePageId)
                    controlPanelStackView.profilePage.readyToSaveDetails = true
                }
            }

            ImportFromBackupPage {
                // import from backup page, index 3
                id: importFromBackupPage

                onLeavePage: {
                    changePageQML(controlPanelStackView.welcomePageStackId)
                }

                onImportAccount: {
                    inputParaObject = {}
                    inputParaObject["archivePath"] = ClientWrapper.utilsAdaptor.getAbsPath(importFromBackupPage.filePath)
                    inputParaObject["password"] = importFromBackupPage.text_passwordFromBackupEditAlias
                    importFromBackupPage.clearAllTextFields()
                    showBackUp = false
                    showBottom = false
                    showProfile = true
                    ClientWrapper.accountAdaptor.createJamiAccount(
                        "", inputParaObject, "", false)
                }
            }

            BackupKeyPage {
                    // backup keys page, index 4
                    id: backupKeysPage

                    onNeverShowAgainBoxClicked: {
                        ClientWrapper.accountAdaptor.settingsNeverShowAgain(isChecked)
                    }

                    onExport_Btn_FileDialogAccepted: {
                        if (accepted) {
                            // is there password? If so, go to password dialog, else, go to following directly
                            if (ClientWrapper.accountAdaptor.hasPassword()) {
                                passwordDialog.path = ClientWrapper.utilsAdaptor.getAbsPath(folderDir)
                                passwordDialog.open()
                                return
                            } else {
                                if (folderDir.length > 0) {
                                    ClientWrapper.accountAdaptor.exportToFile(
                                                ClientWrapper.utilsAdaptor.getCurrAccId(),
                                                ClientWrapper.utilsAdaptor.getAbsPath(folderDir))
                                }
                            }
                        }

                        changePageQML(controlPanelStackView.welcomePageStackId)
                        needToShowMainViewWindow(addedAccountIndex)
                        ClientWrapper.lrcInstance.accountListChanged()
                    }

                    onLeavePage: {
                        changePageQML(controlPanelStackView.welcomePageStackId)
                        needToShowMainViewWindow(addedAccountIndex)
                        ClientWrapper.lrcInstance.accountListChanged()
                    }
            }

            ImportFromDevicePage {
                // import from device page, index 5
                id: importFromDevicePage

                onLeavePage: {
                    changePageQML(controlPanelStackView.welcomePageStackId)
                }

                onImportAccount: {
                    inputParaObject = {}
                    inputParaObject["archivePin"] = importFromDevicePage.text_pinFromDeviceAlias
                    inputParaObject["password"] = importFromDevicePage.text_passwordFromDeviceAlias

                    showProfile = true
                    showBackUp = false
                    showBottom = false
                    ClientWrapper.accountAdaptor.createJamiAccount(
                        "", inputParaObject, "", false)
                }
            }

            ConnectToAccountManagerPage {
                // connect to account manager Page, index 6
                id: connectToAccountManagerPage

                onCreateAccount: {
                    inputParaObject = {}
                    inputParaObject["username"]
                            = connectToAccountManagerPage.text_usernameManagerEditAlias
                    inputParaObject["password"]
                            = connectToAccountManagerPage.text_passwordManagerEditAlias
                    inputParaObject["manager"]
                            = connectToAccountManagerPage.text_accountManagerEditAlias
                    ClientWrapper.accountAdaptor.createJAMSAccount(inputParaObject)
                }

                onLeavePage: {
                    changePageQML(controlPanelStackView.welcomePageStackId)
                }
            }

            SpinnerPage {
                // spinner Page, index 7
                id: spinnerPage
            }

            ProfilePage {
                // profile Page, index 8
                id: profilePage

                function leave() {
                    if (showBackUp)
                        changePageQML(controlPanelStackView.backupKeysPageId)
                    else {
                        changePageQML(controlPanelStackView.welcomePageStackId)
                        needToShowMainViewWindow(addedAccountIndex)
                        ClientWrapper.lrcInstance.accountListChanged()
                    }
                }

                onSaveProfile: {
                    ClientWrapper.settingsAdaptor.setCurrAccAvatar(profilePage.boothImgBase64)
                    ClientWrapper.accountAdaptor.setCurrAccDisplayName(profilePage.displayName)
                    leave()
                }

                onLeavePage: {
                    leave()
                }
            }
        }
    }

    color: JamiTheme.backgroundColor
}
