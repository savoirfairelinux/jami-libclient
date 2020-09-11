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
import net.jami.Adapters 1.0

import "../commoncomponents"
import "../constant"
import "components"

Rectangle {
    id: root

    enum Mode {
        CREATE,
        IMPORT,
        MIGRATE,
        CREATESIP,
        CONNECTMANAGER
    }

    enum WizardViewPageIndex {
        WELCOMEPAGE = 0,
        CREATEACCOUNTPAGE,
        CREATESIPACCOUNTPAGE,
        IMPORTFROMBACKUPPAGE,
        BACKUPKEYSPAGE,
        IMPORTFROMDEVICEPAGE,
        CONNECTTOACCOUNTMANAGERPAGE,
        PROFILEPAGE,
        CREATERENDEZVOUS
    }

    readonly property int layoutSpacing: 12

    property int textFontSize: 9
    property int wizardMode: WizardView.CREATE
    property int addedAccountIndex: -1
    property bool isRdv: false
    property bool showBackUp: false
    property bool showProfile: false
    property bool showBottom: false
    property string fileToImport: ""
    property string registeredName: ""

    property var inputParaObject: ({})

    // signal to redirect the page to main view
    signal needToShowMainViewWindow(int accountIndex)
    signal wizardViewIsClosed

    visible: true
    color: JamiTheme.backgroundColor

    Component.onCompleted: {
        changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
    }

    Connections{
        target: AccountAdapter

        function onAccountAdded(showBackUp, index) {
            addedAccountIndex = index
            AccountAdapter.accountChanged(index)
            if (showProfile) {
                changePageQML(WizardView.WizardViewPageIndex.PROFILEPAGE)
                profilePage.readyToSaveDetails()
                profilePage.isRdv = isRdv
            } else if (controlPanelStackView.currentIndex === WizardView.WizardViewPageIndex.PROFILEPAGE) {
                profilePage.readyToSaveDetails()
                profilePage.isRdv = isRdv
            } else if (showBackUp) {
                changePageQML(WizardView.WizardViewPageIndex.BACKUPKEYSPAGE)
            } else {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                needToShowMainViewWindow(addedAccountIndex)
            }
        }

        // reportFailure
        function onReportFailure() {
            var errorMessage = JamiStrings.errorCreateAccount

            switch(controlPanelStackView.currentIndex) {
            case WizardView.WizardViewPageIndex.IMPORTFROMDEVICEPAGE:
                importFromDevicePage.errorOccured(errorMessage)
                break
            case WizardView.WizardViewPageIndex.IMPORTFROMBACKUPPAGE:
                importFromBackupPage.errorOccured(errorMessage)
                break
            case WizardView.WizardViewPageIndex.CONNECTTOACCOUNTMANAGERPAGE:
                connectToAccountManagerPage.errorOccured(errorMessage)
                break
            }
        }
    }

    function changePageQML(pageIndex) {
        controlPanelStackView.currentIndex = pageIndex
        if (pageIndex === WizardView.WizardViewPageIndex.WELCOMEPAGE) {
            fileToImport = ""
            isRdv = false
            createAccountPage.nameRegistrationUIState = UsernameLineEdit.NameRegistrationState.BLANK
        } else if (pageIndex === WizardView.WizardViewPageIndex.CREATEACCOUNTPAGE) {
            createAccountPage.initializeOnShowUp()
        } else if (pageIndex === WizardView.WizardViewPageIndex.CREATESIPACCOUNTPAGE) {
            createSIPAccountPage.initializeOnShowUp()
        } else if (pageIndex === WizardView.WizardViewPageIndex.IMPORTFROMDEVICEPAGE) {
            importFromDevicePage.initializeOnShowUp()
        } else if (pageIndex === WizardView.WizardViewPageIndex.CONNECTTOACCOUNTMANAGERPAGE) {
            connectToAccountManagerPage.initializeOnShowUp()
        } else if (pageIndex === WizardView.WizardViewPageIndex.IMPORTFROMBACKUPPAGE) {
            importFromBackupPage.clearAllTextFields()
            fileToImport = ""
        } else if (pageIndex === WizardView.WizardViewPageIndex.PROFILEPAGE) {
            profilePage.initializeOnShowUp()
            profilePage.showBottom = showBottom
        } else if (pageIndex === WizardView.WizardViewPageIndex.CREATERENDEZVOUS) {
            isRdv = true
            controlPanelStackView.currentIndex = WizardView.WizardViewPageIndex.CREATEACCOUNTPAGE
            createAccountPage.initializeOnShowUp(true)
        }
    }

    PasswordDialog {
        id: passwordDialog

        anchors.centerIn: parent.Center

        visible: false
        purpose: PasswordDialog.ExportAccount

        onDoneSignal: {
            if (currentPurpose === passwordDialog.ExportAccount) {
                var success = (code === successCode)

                var title = success ? qsTr("Success") : qsTr("Error")
                var info = success ? JamiStrings.backupSuccessful : JamiStrings.backupFailed

                AccountAdapter.passwordSetStatusMessageBox(success,
                                                         title, info)
                if (success) {
                    console.log("Account Export Succeed")
                    needToShowMainViewWindow(addedAccountIndex)
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: forceActiveFocus()
    }

    StackLayout {
        id: controlPanelStackView

        anchors.fill: parent

        currentIndex: WizardView.WizardViewPageIndex.WELCOMEPAGE

        WelcomePage {
            id: welcomePage

            onWelcomePageRedirectPage: {
                changePageQML(toPageIndex)
            }

            onLeavePage: {
                wizardViewIsClosed()
            }
        }

        CreateAccountPage {
            id: createAccountPage

            onCreateAccount: {
                inputParaObject = {}
                inputParaObject["isRendezVous"] = isRdv
                inputParaObject["password"] = text_passwordEditAlias
                AccountAdapter.createJamiAccount(
                    createAccountPage.text_usernameEditAlias,
                    inputParaObject,
                    createAccountPage.boothImgBase64,
                    true)
                showBackUp = !isRdv
                showBottom = true
                changePageQML(WizardView.WizardViewPageIndex.PROFILEPAGE)
            }

            onLeavePage: {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
            }
        }

        CreateSIPAccountPage {
            id: createSIPAccountPage

            onLeavePage: {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
            }

            onCreateAccount: {
                inputParaObject = {}
                inputParaObject["hostname"] = createSIPAccountPage.text_sipServernameEditAlias
                inputParaObject["username"] = createSIPAccountPage.text_sipUsernameEditAlias
                inputParaObject["password"] = createSIPAccountPage.text_sipPasswordEditAlias
                inputParaObject["proxy"] = createSIPAccountPage.text_sipProxyEditAlias
                createSIPAccountPage.clearAllTextFields()

                AccountAdapter.createSIPAccount(inputParaObject, "")
                showBackUp = false
                showBottom = false
                changePageQML(WizardView.WizardViewPageIndex.PROFILEPAGE)
                controlPanelStackView.profilePage.readyToSaveDetails()
            }
        }

        ImportFromBackupPage {
            id: importFromBackupPage

            onLeavePage: {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
            }

            onImportAccount: {
                inputParaObject = {}
                inputParaObject["archivePath"] = UtilsAdapter.getAbsPath(importFromBackupPage.filePath)
                inputParaObject["password"] = importFromBackupPage.text_passwordFromBackupEditAlias
                showBackUp = false
                showBottom = false
                showProfile = true
                AccountAdapter.createJamiAccount(
                    "", inputParaObject, "", false)
            }
        }

        BackupKeyPage {
            id: backupKeysPage

            onNeverShowAgainBoxClicked: {
                SettingsAdapter.setValue(Settings.NeverShowMeAgain, isChecked)
            }

            onExport_Btn_FileDialogAccepted: {
                if (accepted) {
                    // is there password? If so, go to password dialog, else, go to following directly
                    if (AccountAdapter.hasPassword()) {
                        passwordDialog.path = UtilsAdapter.getAbsPath(folderDir)
                        passwordDialog.open()
                        return
                    } else {
                        if (folderDir.length > 0) {
                            AccountAdapter.exportToFile(
                                        UtilsAdapter.getCurrAccId(),
                                        UtilsAdapter.getAbsPath(folderDir))
                        }
                    }
                }

                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                needToShowMainViewWindow(addedAccountIndex)
            }

            onLeavePage: {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                needToShowMainViewWindow(addedAccountIndex)
            }
        }

        ImportFromDevicePage {
            id: importFromDevicePage

            onLeavePage: {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
            }

            onImportAccount: {
                inputParaObject = {}
                inputParaObject["archivePin"] = importFromDevicePage.text_pinFromDeviceAlias
                inputParaObject["password"] = importFromDevicePage.text_passwordFromDeviceAlias

                showProfile = true
                showBackUp = false
                showBottom = false
                AccountAdapter.createJamiAccount(
                    "", inputParaObject, "", false)
            }
        }

        ConnectToAccountManagerPage {
            id: connectToAccountManagerPage

            onCreateAccount: {
                inputParaObject = {}
                inputParaObject["username"]
                        = connectToAccountManagerPage.text_usernameManagerEditAlias
                inputParaObject["password"]
                        = connectToAccountManagerPage.text_passwordManagerEditAlias
                inputParaObject["manager"]
                        = connectToAccountManagerPage.text_accountManagerEditAlias
                AccountAdapter.createJAMSAccount(inputParaObject)
            }

            onLeavePage: {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
            }
        }

        ProfilePage {
            id: profilePage

            function leave() {
                if (showBackUp)
                    changePageQML(WizardView.WizardViewPageIndex.BACKUPKEYSPAGE)
                else {
                    changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                    needToShowMainViewWindow(addedAccountIndex)
                }
            }

            onSaveProfile: {
                SettingsAdapter.setCurrAccAvatar(profilePage.boothImgBase64)
                AccountAdapter.setCurrAccDisplayName(profilePage.displayName)
                leave()
            }

            onLeavePage: {
                leave()
            }
        }
    }
}
