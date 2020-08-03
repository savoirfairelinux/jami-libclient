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
import QtQuick.Window 2.14
import QtQuick.Controls 1.4 as CT
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "../commoncomponents"
import "../constant"
import "components"

Window {
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

    property int layoutWidth: 768
    property int layoutHeight: 768
    property int textFontSize: 9
    property int wizardMode: WizardView.CREATE
    property int addedAccountIndex: -1
    property bool registrationStateOk: false
    property string fileToImport: ""
    property string registedName: ""

    property var inputParaObject: ({})

    /*
     * signal to redirect the page to main view
     */
    signal needToShowMainViewWindow(int accountIndex)
    signal wizardViewIsClosed

    title: "Jami"
    visible: true
    width: layoutWidth
    height: layoutHeight

    onClosing: {
        close.accepted = false
        changePageQML(controlPanelStackView.welcomePageStackId)
        wizardViewWindow.hide()
        wizardViewWindow.wizardViewIsClosed()
    }

    Component.onCompleted: {
        changePageQML(
                    controlPanelStackView.welcomePageStackId)
    }

    Connections{
        target: ClientWrapper.accountAdaptor

        function onAccountAdded(showBackUp, index) {
            addedAccountIndex = index
            if (showBackUp) {
                changePageQML(controlPanelStackView.backupKeysPageId)
            } else {
                wizardViewWindow.hide()
                changePageQML(controlPanelStackView.welcomePageStackId)
                needToShowMainViewWindow(addedAccountIndex)
                ClientWrapper.lrcInstance.accountListChanged()
            }
        }

        // reportFailure
        function onReportFailure() {
            reportFailureQML()
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

    // failure redirect timer and qml object holder
    Timer {
        id: failureRedirectPageTimer

        repeat: false
        triggeredOnStart: false
        interval: 1000

        onTriggered: {
            spinnerPage.successState = true
        }
    }

    function reportFailureQML() {
        spinnerPage.successState = false
        failureRedirectPageTimer.restart()
    }

    function createAccountQML() {
        switch (wizardMode) {
        case WizardView.CONNECTMANAGER:
            ClientWrapper.accountAdaptor.createJAMSAccount(inputParaObject)
            break
        case WizardView.CREATE:
        case WizardView.IMPORT:
            ClientWrapper.accountAdaptor.createJamiAccount(registedName,
                                                           inputParaObject,
                                                           createAccountPage.boothImgBase64,
                                                           (wizardMode === WizardView.CREATE))
            break
        default:
            ClientWrapper.accountAdaptor.createSIPAccount(inputParaObject,createSIPAccountPage.boothImgBase64)
        }

        changePageQML(controlPanelStackView.spinnerPageId)
        update()
    }

    function slotRegisteredNameFound(status, address, name) {
        if (name.length < 3) {
            registrationStateOk = false
            createAccountPage.nameRegistrationUIState = WizardView.INVALID
        } else if (registedName === name) {
            switch (status) {
            case NameDirectory.LookupStatus.NOT_FOUND:
            case NameDirectory.LookupStatus.ERROR:
                registrationStateOk = true
                createAccountPage.nameRegistrationUIState = WizardView.FREE
                break
            case NameDirectory.LookupStatus.INVALID_NAME:
            case NameDirectory.LookupStatus.INVALID:
                registrationStateOk = false
                createAccountPage.nameRegistrationUIState = WizardView.INVALID
                break
            case NameDirectory.LookupStatus.SUCCESS:
                registrationStateOk = false
                createAccountPage.nameRegistrationUIState = WizardView.TAKEN
                break
            }
        }
        validateWizardProgressionQML()
    }

    // function to set up nav bar visibility and the three buttons' visibiliy
    function setNavBarVisibility(navVisible, back) {
        navBarView.visible = (navVisible == true) || (back == true)
        btnNext.visible = (navVisible == true)
        btnPevious.visible = (navVisible == true)
        btnBack.visible = (back == true)
                && (ClientWrapper.utilsAdaptor.getAccountListSize() != 0)
    }

    function processWizardInformationsQML() {
        inputParaObject = {}
        switch (wizardMode) {
        case WizardView.CREATE:
            spinnerPage.progressLabelEditText = qsTr(
                        "Generating your Jami account...")
            inputParaObject["alias"] = createAccountPage.text_fullNameEditAlias

            inputParaObject["password"] = createAccountPage.text_confirmPasswordEditAlias

            createAccountPage.clearAllTextFields()
            break
        case WizardView.IMPORT:
            registedName = ""
            spinnerPage.progressLabelEditText = qsTr(
                        "Importing account archive...")
            // should only work in import from backup page or import from device page
            if (controlPanelStackView.currentIndex
                    == controlPanelStackView.importFromBackupPageId) {
                inputParaObject["password"]
                        = importFromBackupPage.text_passwordFromDeviceAlias
                importFromBackupPage.clearAllTextFields()
            } else if (controlPanelStackView.currentIndex
                       == controlPanelStackView.importFromDevicePageId) {
                inputParaObject["archivePin"] = importFromBackupPage.text_pinFromDeviceAlias
                inputParaObject["password"]
                        = importFromDevicePage.text_passwordFromDeviceAlias
                importFromDevicePage.clearAllTextFields()
            }
            break
        case WizardView.MIGRATE:
            spinnerPage.progressLabelEditText = qsTr(
                        "Migrating your Jami account...")
            break
        case WizardView.CREATESIP:
            spinnerPage.progressLabelEditText = qsTr(
                        "Generating your SIP account...")
            if (createSIPAccountPage.text_sipFullNameEditAlias.length == 0) {
                inputParaObject["alias"] = "SIP"
            } else {
                inputParaObject["alias"] = createSIPAccountPage.text_sipFullNameEditAlias
            }

            inputParaObject["hostname"] = createSIPAccountPage.text_sipServernameEditAlias
            inputParaObject["username"] = createSIPAccountPage.text_sipUsernameEditAlias
            inputParaObject["password"] = createSIPAccountPage.text_sipPasswordEditAlias
            inputParaObject["proxy"] = createSIPAccountPage.text_sipProxyEditAlias

            break
        case WizardView.CONNECTMANAGER:
            spinnerPage.progressLabelEditText = qsTr(
                        "Connecting to account manager...")
            inputParaObject["username"]
                    = connectToAccountManagerPage.text_usernameManagerEditAlias
            inputParaObject["password"]
                    = connectToAccountManagerPage.text_passwordManagerEditAlias
            inputParaObject["manager"]
                    = connectToAccountManagerPage.text_accountManagerEditAlias
            connectToAccountManagerPage.clearAllTextFields()
            break
        }

        inputParaObject["archivePath"] = fileToImport

        if (!("archivePin" in inputParaObject)) {
            inputParaObject["archivePath"] = ""
        }

        // change page to spinner page
        changePageQML(controlPanelStackView.spinnerPageId)
        //create account
        createAccountQML()
        ClientWrapper.utilsAdaptor.createStartupLink()
    }

    function changePageQML(pageIndex) {
        if (pageIndex == controlPanelStackView.spinnerPageId) {
            setNavBarVisibility(false)
        }
        controlPanelStackView.currentIndex = pageIndex
        if (pageIndex == controlPanelStackView.welcomePageStackId) {
            fileToImport = ""
            setNavBarVisibility(false, true)
            createAccountPage.nameRegistrationUIState = WizardView.BLANK
        } else if (pageIndex == controlPanelStackView.createAccountPageId) {
            createAccountPage.initializeOnShowUp()
            setNavBarVisibility(true)
            // connection between register name found and its slot
            registeredNameFoundConnection.enabled = true
            // validate wizard progression
            validateWizardProgressionQML()
            // start photobooth
            createAccountPage.startBooth()
        } else if (pageIndex == controlPanelStackView.createSIPAccountPageId) {
            createSIPAccountPage.initializeOnShowUp()
            setNavBarVisibility(true)
            btnNext.enabled = true
            // start photo booth
            createSIPAccountPage.startBooth()
        } else if (pageIndex == controlPanelStackView.importFromDevicePageId) {
            importFromDevicePage.initializeOnShowUp()
            setNavBarVisibility(true)
        } else if (pageIndex == controlPanelStackView.spinnerPageId) {
            createAccountPage.nameRegistrationUIState = WizardView.BLANK
            createAccountPage.isToSetPassword_checkState_choosePasswordCheckBox = false
        } else if (pageIndex == controlPanelStackView.connectToAccountManagerPageId) {
            setNavBarVisibility(true)
            connectToAccountManagerPage.initializeOnShowUp()
            btnNext.enabled = false
        } else if (pageIndex == controlPanelStackView.importFromBackupPageId) {
            setNavBarVisibility(true)
            importFromBackupPage.clearAllTextFields()
            fileToImport = ""
            btnNext.enabled = false
        } else if (pageIndex == controlPanelStackView.backupKeysPageId) {
            setNavBarVisibility(false)
        }
    }

    function validateWizardProgressionQML() {
        if (controlPanelStackView.currentIndex
                == controlPanelStackView.importFromDevicePageId) {
            var validPin = !(importFromDevicePage.text_pinFromDeviceAlias.length == 0)
            btnNext.enabled = validPin
            return
        } else if (controlPanelStackView.currentIndex
                   == controlPanelStackView.connectToAccountManagerPageId) {
            var validUsername = !(connectToAccountManagerPage.text_usernameManagerEditAlias.length == 0)
            var validPassword = !(connectToAccountManagerPage.text_passwordManagerEditAlias.length == 0)
            var validManager = !(connectToAccountManagerPage.text_accountManagerEditAlias.length == 0)
            btnNext.enabled = validUsername && validPassword
                    && validManager
            return
        } else if (controlPanelStackView.currentIndex
                   == controlPanelStackView.importFromBackupPageId) {
            var validImport = !(fileToImport.length == 0)
            btnNext.enabled = validImport
            return
        }

        var usernameOk = !createAccountPage.checkState_signUpCheckboxAlias
                || (createAccountPage.checkState_signUpCheckboxAlias
                    && !(registedName.length == 0)
                    && (registedName == createAccountPage.text_usernameEditAlias)
                    && (registrationStateOk == true))
        var passwordOk = (createAccountPage.text_passwordEditAlias
                          == createAccountPage.text_confirmPasswordEditAlias)

        // set password status label
        if (passwordOk
                && !(createAccountPage.text_passwordEditAlias.length == 0)) {
            createAccountPage.displayState_passwordStatusLabelAlias = "Success"
        } else if (!passwordOk) {
            createAccountPage.displayState_passwordStatusLabelAlias = "Fail"
        } else {
            createAccountPage.displayState_passwordStatusLabelAlias = "Hide"
        }
        //set enable state of next button
        btnNext.enabled = (usernameOk && passwordOk)
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
                    wizardViewWindow.hide()
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

    // main frame rectangle
    ScrollView  {
        id: wizardViewRect
        anchors.fill: parent

        clip: true

        ColumnLayout {
                id: content
                width: wizardViewWindow.width      // ensure correct width
                height: wizardViewWindow.height     // ensure correct height

                Layout.alignment: Qt.AlignHCenter
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    StackLayout {
                        id: controlPanelStackView
                        currentIndex: welcomePageStackId
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        property int welcomePageStackId: 0
                        property int createAccountPageId: 1
                        property int createSIPAccountPageId: 2
                        property int importFromBackupPageId: 3
                        property int backupKeysPageId: 4
                        property int importFromDevicePageId: 5
                        property int connectToAccountManagerPageId: 6
                        property int spinnerPageId: 7

                        WelcomePageLayout {
                            // welcome page, index 0
                            id: welcomePage

                            onWelcomePageRedirectPage: {
                                changePageQML(toPageIndex)
                            }

                            onVisibleChanged: {
                                if (visible)
                                    setNavBarVisibility(false,
                                                                          true)
                            }

                            Component.onCompleted: {
                                setNavBarVisibility(false, true)
                            }
                        }

                        CreateAccountPage {
                            // create account page, index 1
                            id: createAccountPage

                            onText_usernameEditAliasChanged: {
                            registrationStateOk = false
                            if (createAccountPage.checkState_signUpCheckboxAlias
                                    && (createAccountPage.text_usernameEditAlias.length != 0)) {
                                registedName = ClientWrapper.utilsAdaptor.stringSimplifier(
                                            createAccountPage.text_usernameEditAlias)
                                lookupTimer.restart()
                                } else {
                                createAccountPage.nameRegistrationUIState = WizardView.BLANK
                                lookupTimer.stop()
                                if (createAccountPage.text_usernameEditAlias.length == 0) {
                                    lookupTimer.restart()
                                    }
                                }
                                validateWizardProgressionQML()
                            }

                            onValidateWizardProgressionCreateAccountPage: {
                                validateWizardProgressionQML()
                            }

                            onText_passwordEditAliasChanged: {
                                validateWizardProgressionQML()
                            }

                            onText_confirmPasswordEditAliasChanged: {
                                validateWizardProgressionQML()
                            }

                        Timer {
                            id: lookupTimer

                            repeat: false
                            triggeredOnStart: false
                            interval: 200

                            onTriggered: {
                                if (createAccountPage.checkState_signUpCheckboxAlias
                                        && (createAccountPage.text_usernameEditAlias.length != 0)) {
                                    createAccountPage.nameRegistrationUIState = WizardView.SEARCHING
                                    ClientWrapper.nameDirectory.lookupName("", registedName)
                                }
                            }
                        }
                    }

                        CreateSIPAccountPage {
                            // create SIP account page, index 2
                            id: createSIPAccountPage
                        }

                        ImportFromBackupPage {
                            // import from backup page, index 3
                            id: importFromBackupPage

                            onText_passwordFromBackupEditAliasChanged: {
                                validateWizardProgressionQML()
                            }

                        onImportFromFile_Dialog_Accepted: {
                            fileToImport = ClientWrapper.utilsAdaptor.toNativeSeparators(fileDir)
                            inputParaObject[""]

                            if (fileToImport.length != 0) {
                                importFromBackupPage.fileImportBtnText = ClientWrapper.utilsAdaptor.toFileInfoName(
                                            fileToImport)
                            } else {
                                importFromBackupPage.fileImportBtnText = qsTr(
                                            "Archive(none)")
                            }
                            validateWizardProgressionQML()
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

                                wizardViewWindow.hide()
                                changePageQML(controlPanelStackView.welcomePageStackId)
                                needToShowMainViewWindow(addedAccountIndex)
                                ClientWrapper.lrcInstance.accountListChanged()
                            }


                            onSkip_Btn_Clicked: {
                                wizardViewWindow.hide()
                                changePageQML(controlPanelStackView.welcomePageStackId)
                                needToShowMainViewWindow(addedAccountIndex)
                                ClientWrapper.lrcInstance.accountListChanged()
                            }
                    }

                        ImportFromDevicePage {
                            // import from device page, index 5
                            id: importFromDevicePage

                            onText_pinFromDeviceAliasChanged: {
                                validateWizardProgressionQML()
                            }

                            onText_passwordFromDeviceAliasChanged: {
                                validateWizardProgressionQML()
                            }
                        }

                        ConnectToAccountManagerPage {
                            // connect to account manager Page, index 6
                            id: connectToAccountManagerPage

                            onText_usernameManagerEditAliasChanged: {
                                validateWizardProgressionQML()
                            }

                            onText_passwordManagerEditAliasChanged: {
                                validateWizardProgressionQML()
                            }

                            onText_accountManagerEditAliasChanged: {
                                validateWizardProgressionQML()
                            }
                        }

                        SpinnerPage {
                            // spinner Page, index 7
                            id: spinnerPage
                        }
                    }
                }

                RowLayout {
                    id: navBarView

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    Layout.fillWidth: true

                    Layout.maximumHeight: 52
                    Layout.preferredHeight: 52

                    HoverableGradientButton {
                        id: btnPevious
                        Layout.alignment: Qt.AlignLeft
                        width: 85
                        height: 30
                        radius: height / 2

                        Layout.leftMargin: 11

                        Layout.preferredWidth: 85
                        Layout.preferredHeight: 30
                        text: qsTr("Previous")
                        font.pointSize: 10
                        font.kerning: true

                        onClicked: {
                            // stop photobooth previewing
                            if(controlPanelStackView.currentIndex == controlPanelStackView.createAccountPageId) {
                                createAccountPage.stopBooth()
                            }
                            if(controlPanelStackView.currentIndex == controlPanelStackView.createSIPAccountPageId) {
                                createSIPAccountPage.stopBooth()
                            }

                            // Disconnect registered name found Connection
                            registeredNameFoundConnection.enabled = false
                            // deal with look up status label and collapsible password widget
                            createAccountPage.nameRegistrationUIState = WizardView.BLANK
                            createAccountPage.isToSetPassword_checkState_choosePasswordCheckBox = false
                            // switch to welcome page
                            if (controlPanelStackView.currentIndex
                                    == controlPanelStackView.createAccountPageId
                                    || controlPanelStackView.currentIndex
                                    == controlPanelStackView.createSIPAccountPageId
                                    || controlPanelStackView.currentIndex
                                    == controlPanelStackView.importFromBackupPageId
                                    || controlPanelStackView.currentIndex
                                    == controlPanelStackView.importFromDevicePageId
                                    || controlPanelStackView.currentIndex
                                    == controlPanelStackView.connectToAccountManagerPageId) {
                                changePageQML(
                                            controlPanelStackView.welcomePageStackId)
                            }
                        }
                    }

                    Item {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: true
                        Layout.minimumWidth: 40
                        Layout.fillWidth: true
                    }

                    HoverableGradientButton {
                        id: btnBack
                        Layout.alignment: Qt.AlignLeft
                        width: 85
                        height: 30
                        radius: height / 2

                        Layout.preferredWidth: 85
                        Layout.preferredHeight: 30
                        text: qsTr("Back")
                        font.pointSize: 10
                        font.kerning: true

                        onClicked: {
                            wizardViewWindow.hide()
                            needToShowMainViewWindow(addedAccountIndex)
                        }
                    }

                    Item {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: true
                        Layout.minimumWidth: 40
                        Layout.fillWidth: true
                    }

                    HoverableGradientButton {
                        id: btnNext
                        Layout.alignment: Qt.AlignRight
                        width: 85
                        height: 30
                        radius: height / 2

                        Layout.rightMargin: 11

                        Layout.minimumWidth: 85
                        Layout.preferredWidth: 85
                        Layout.maximumWidth: 85

                        Layout.minimumHeight: 30
                        Layout.preferredHeight: 30
                        Layout.maximumHeight: 30

                        text: qsTr("Next")
                        font.pointSize: 10
                        font.kerning: true

                        onClicked: {
                            // stop photobooth previewing
                            if(controlPanelStackView.currentIndex == controlPanelStackView.createAccountPageId) {
                                createAccountPage.stopBooth()
                            }
                            if(controlPanelStackView.currentIndex == controlPanelStackView.createSIPAccountPageId) {
                                createSIPAccountPage.stopBooth()
                            }

                            registeredNameFoundConnection.enabled = false

                            if (controlPanelStackView.currentIndex
                                    == controlPanelStackView.createAccountPageId) {
                                wizardMode = WizardView.CREATE
                                processWizardInformationsQML()
                            } else if (controlPanelStackView.currentIndex
                                       == controlPanelStackView.importFromDevicePageId) {
                                wizardMode = WizardView.IMPORT
                                processWizardInformationsQML()
                            } else if (controlPanelStackView.currentIndex
                                       == controlPanelStackView.createSIPAccountPageId) {
                                wizardMode = WizardView.CREATESIP
                                processWizardInformationsQML()
                            } else if (controlPanelStackView.currentIndex
                                       == controlPanelStackView.connectToAccountManagerPageId) {
                                wizardMode = WizardView.CONNECTMANAGER
                                processWizardInformationsQML()
                            } else if (controlPanelStackView.currentIndex
                                       == controlPanelStackView.importFromBackupPageId) {
                                wizardMode = WizardView.IMPORT
                                processWizardInformationsQML()
                            }
                        }
                    }
                }
            }
    }
}
