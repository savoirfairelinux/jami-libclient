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
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../"
import "../commoncomponents"
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
    readonly property int backButtonMargins: 20

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
    signal loaderSourceChangeRequested(int sourceToLoad)
    signal wizardViewIsClosed

    visible: true
    color: JamiTheme.backgroundColor

    Component.onCompleted: {
        changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
    }

    Connections{
        target: AccountAdapter

        enabled: controlPanelStackView.currentIndex !== WizardView.WizardViewPageIndex.WELCOMEPAGE

        function onAccountAdded(accountId, showBackUp, index) {
            addedAccountIndex = index
            AccountAdapter.changeAccount(index)
            if (showProfile) {
                changePageQML(WizardView.WizardViewPageIndex.PROFILEPAGE)
                profilePage.readyToSaveDetails()
                profilePage.isRdv = isRdv
                profilePage.createdAccountId = accountId
            } else if (controlPanelStackView.currentIndex === WizardView.WizardViewPageIndex.PROFILEPAGE) {
                profilePage.readyToSaveDetails()
                profilePage.isRdv = isRdv
                profilePage.createdAccountId = accountId
            } else if (showBackUp) {
                changePageQML(WizardView.WizardViewPageIndex.BACKUPKEYSPAGE)
            } else {
                changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                loaderSourceChangeRequested(MainApplicationWindow.LoadedSource.MainView)
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
            createAccountPage.initializeOnShowUp(false)
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

        visible: false
        purpose: PasswordDialog.ExportAccount

        onDoneSignal: {
            if (currentPurpose === passwordDialog.ExportAccount) {
                var title = success ? qsTr("Success") : qsTr("Error")
                var info = success ? JamiStrings.backupSuccessful : JamiStrings.backupFailed

                AccountAdapter.passwordSetStatusMessageBox(success,
                                                         title, info)
                if (success) {
                    console.log("Account Export Succeed")
                    loaderSourceChangeRequested(MainApplicationWindow.LoadedSource.MainView)
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: forceActiveFocus()
    }

    ScrollView {
        id: wizardViewScrollView

        property ScrollBar vScrollBar: ScrollBar.vertical

        anchors.fill: parent

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        clip: true
        contentHeight: controlPanelStackView.height

        StackLayout {
            id: controlPanelStackView

            anchors.centerIn: parent

            width: wizardViewScrollView.width

            currentIndex: WizardView.WizardViewPageIndex.WELCOMEPAGE

            Component.onCompleted: {
                // avoid binding loop
                height = Qt.binding(function (){
                    var index = currentIndex
                            === WizardView.WizardViewPageIndex.CREATERENDEZVOUS ?
                                WizardView.WizardViewPageIndex.CREATEACCOUNTPAGE : currentIndex
                    return Math.max(
                                controlPanelStackView.itemAt(index).preferredHeight,
                                wizardViewScrollView.height)
                })
            }

            WelcomePage {
                id: welcomePage

                Layout.alignment: Qt.AlignCenter

                onWelcomePageRedirectPage: {
                    changePageQML(toPageIndex)
                }

                onLeavePage: {
                    wizardViewIsClosed()
                }

                onScrollToBottom: {
                    if (welcomePage.preferredHeight > root.height)
                        wizardViewScrollView.vScrollBar.position = 1
                }
            }

            CreateAccountPage {
                id: createAccountPage

                Layout.alignment: Qt.AlignCenter

                onCreateAccount: {
                    inputParaObject = {}
                    inputParaObject["isRendezVous"] = isRdv
                    inputParaObject["password"] = text_passwordEditAlias
                    AccountAdapter.createJamiAccount(
                        createAccountPage.text_usernameEditAlias,
                        inputParaObject,
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

                Layout.alignment: Qt.AlignCenter

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

                Layout.alignment: Qt.AlignCenter

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

                Layout.alignment: Qt.AlignCenter

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
                                            LRCInstance.currentAccountId,
                                            UtilsAdapter.getAbsPath(folderDir))
                            }
                        }
                    }

                    changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                    loaderSourceChangeRequested(MainApplicationWindow.LoadedSource.MainView)
                }

                onLeavePage: {
                    changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                    loaderSourceChangeRequested(MainApplicationWindow.LoadedSource.MainView)
                }
            }

            ImportFromDevicePage {
                id: importFromDevicePage

                Layout.alignment: Qt.AlignCenter

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

                Layout.alignment: Qt.AlignCenter

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

                Layout.alignment: Qt.AlignCenter

                function leave() {
                    if (showBackUp)
                        changePageQML(WizardView.WizardViewPageIndex.BACKUPKEYSPAGE)
                    else {
                        changePageQML(WizardView.WizardViewPageIndex.WELCOMEPAGE)
                        loaderSourceChangeRequested(MainApplicationWindow.LoadedSource.MainView)
                    }

                    profilePage.initializeOnShowUp()
                }

                onSaveProfile: {
                    AccountAdapter.setCurrAccDisplayName(profilePage.displayName)
                    leave()
                }

                onLeavePage: leave()
            }
        }
    }
}
