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
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import Qt.labs.platform 1.1

import "../"
import "../../constant"
import "../../commoncomponents"
import "../../settingsview/components"

Rectangle {
    id: root

    property alias text_usernameEditAlias: usernameEdit.text
    property alias nameRegistrationUIState: usernameEdit.nameRegistrationState
    property bool isRendezVous: false
    property alias text_passwordEditAlias: passwordEdit.text
    property int preferredHeight: {
        if (createAccountStack.currentIndex === 0)
            return usernameColumnLayout.implicitHeight
        return passwordColumnLayout.implicitHeight
    }

    signal createAccount
    signal leavePage

    function initializeOnShowUp(isRdv) {
        isRendezVous = isRdv
        createAccountStack.currentIndex = 0
        clearAllTextFields()
        passwordSwitch.checked = false
    }

    function clearAllTextFields() {
        usernameEdit.clear()
        passwordEdit.clear()
        passwordConfirmEdit.clear()
    }

    color: JamiTheme.backgroundColor

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Esc"
        enabled: !root.activeFocus
        onActivated: leavePage()
    }

    onVisibleChanged: {
        if (visible && createAccountStack.currentIndex === 0)
            usernameEdit.focus = true
    }

    // JamiFileDialog for exporting account
    JamiFileDialog {
        id: exportBtn_Dialog

        mode: JamiFileDialog.SaveFile

        title: JamiStrings.backupAccountHere
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            export_Btn_FileDialogAccepted(true, file)
        }

        onRejected: {
            export_Btn_FileDialogAccepted(false, folder)
        }

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }
    }

    StackLayout {
        id: createAccountStack

        anchors.fill: parent

        currentIndex: 0

        Rectangle {
            color: JamiTheme.backgroundColor

            ColumnLayout {
                id: usernameColumnLayout

                spacing: layoutSpacing

                anchors.centerIn: parent

                width: root.width

                RowLayout {
                    spacing: layoutSpacing

                    Layout.alignment: Qt.AlignCenter
                    Layout.topMargin: backButtonMargins
                    Layout.preferredWidth: usernameEdit.width

                    Label {
                        text: isRendezVous ? JamiStrings.chooseNameRV : qsTr("Choose a username for your account")
                        font.pointSize: JamiTheme.textFontSize + 3
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight

                        text: JamiStrings.recommended
                        color: "white"
                        padding: 8

                        background: Rectangle {
                            color: "#aed581"
                            radius: 24
                            anchors.fill: parent
                        }
                    }
                }

                UsernameLineEdit {
                    id: usernameEdit

                    Layout.topMargin: 15
                    Layout.preferredHeight: fieldLayoutHeight
                    Layout.preferredWidth:  chooseUsernameButton.width
                    Layout.alignment: Qt.AlignHCenter

                    placeholderText: isRendezVous ? qsTr("Choose a name") : qsTr("Choose your username")
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: text.length !==0

                    text: {
                        switch(nameRegistrationUIState){
                        case UsernameLineEdit.NameRegistrationState.BLANK:
                        case UsernameLineEdit.NameRegistrationState.SEARCHING:
                        case UsernameLineEdit.NameRegistrationState.FREE:
                            return ""
                        case UsernameLineEdit.NameRegistrationState.INVALID:
                            return isRendezVous ? qsTr("Invalid name") : qsTr("Invalid username")
                        case UsernameLineEdit.NameRegistrationState.TAKEN:
                            return isRendezVous ? qsTr("Name already taken") : qsTr("Username already taken")
                        }
                    }
                    font.pointSize: JamiTheme.textFontSize
                    color: "red"
                }

                MaterialButton {
                    id: chooseUsernameButton

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredWidth
                    Layout.preferredHeight: preferredHeight

                    fontCapitalization: Font.AllUppercase
                    text: isRendezVous ? JamiStrings.chooseName : JamiStrings.chooseUsername
                    enabled: nameRegistrationUIState === UsernameLineEdit.NameRegistrationState.FREE
                    color: nameRegistrationUIState === UsernameLineEdit.NameRegistrationState.FREE ?
                               JamiTheme.wizardBlueButtons :
                               JamiTheme.buttonTintedGreyInactive
                    hoveredColor: JamiTheme.buttonTintedBlueHovered
                    pressedColor: JamiTheme.buttonTintedBluePressed

                    onClicked: {
                        if (nameRegistrationUIState === UsernameLineEdit.NameRegistrationState.FREE)
                            createAccountStack.currentIndex = createAccountStack.currentIndex + 1
                    }
                }

                MaterialButton {
                    id: skipButton

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredWidth
                    Layout.preferredHeight: preferredHeight

                    text: JamiStrings.skip
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed
                    outlined: true

                    onClicked: {
                        usernameEdit.clear()
                        createAccountStack.currentIndex =
                                createAccountStack.currentIndex + 1
                    }
                }

                AccountCreationStepIndicator {
                    Layout.topMargin: backButtonMargins
                    Layout.bottomMargin: backButtonMargins
                    Layout.alignment: Qt.AlignHCenter

                    spacing: layoutSpacing
                    steps: 3
                    currentStep: 1
                }
            }
        }

        Rectangle {
            color: JamiTheme.backgroundColor

            ColumnLayout {
                id: passwordColumnLayout

                spacing: layoutSpacing

                anchors.centerIn: parent
                width: root.width

                RowLayout {
                    spacing: layoutSpacing

                    Layout.alignment: Qt.AlignCenter
                    Layout.topMargin: backButtonMargins
                    Layout.preferredWidth: usernameEdit.width

                    Label {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: JamiStrings.createPassword
                        font.pointSize: JamiTheme.textFontSize + 3
                    }

                    Switch {
                        id: passwordSwitch

                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        Layout.leftMargin:  -layoutSpacing
                        Layout.topMargin: 5
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                        text: JamiStrings.optional
                        color: "white"
                        padding: 8

                        background: Rectangle {
                            color: "#28b1ed"
                            radius: 24
                            anchors.fill: parent
                        }
                    }
                }

                MaterialLineEdit {
                    id: passwordEdit

                    Layout.preferredHeight: fieldLayoutHeight
                    Layout.preferredWidth: createAccountButton.width
                    Layout.alignment: Qt.AlignHCenter

                    visible: passwordSwitch.checked

                    selectByMouse: true
                    echoMode: TextInput.Password
                    placeholderText: JamiStrings.password
                    font.pointSize: 9
                    font.kerning: true
                }

                MaterialLineEdit {
                    id: passwordConfirmEdit

                    Layout.preferredHeight: fieldLayoutHeight
                    Layout.preferredWidth: createAccountButton.width
                    Layout.alignment: Qt.AlignHCenter

                    visible: passwordSwitch.checked

                    selectByMouse: true
                    echoMode: TextInput.Password
                    placeholderText: JamiStrings.confirmPassword
                    font.pointSize: 9
                    font.kerning: true
                }

                Label {
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: createAccountButton.width - 10
                    Layout.leftMargin: (root.width - createAccountButton.width) / 2

                    text: JamiStrings.notePasswordRecovery
                    wrapMode: Text.WordWrap
                    font.pointSize: JamiTheme.textFontSize

                    onFontChanged: Layout.preferredHeight =
                                   JamiQmlUtils.getTextBoundingRect(font, text).height * 2
                }

                MaterialButton {
                    id: createAccountButton

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredWidth
                    Layout.preferredHeight: preferredHeight

                    function checkEnable() {
                        return !passwordSwitch.checked ||
                                (passwordEdit.text === passwordConfirmEdit.text
                                 && passwordEdit.text.length !== 0)
                    }

                    fontCapitalization: Font.AllUppercase
                    text: isRendezVous ? JamiStrings.createRV : JamiStrings.createAccount
                    enabled: checkEnable()
                    color: checkEnable() ? JamiTheme.wizardBlueButtons :
                                           JamiTheme.buttonTintedGreyInactive
                    hoveredColor: JamiTheme.buttonTintedBlueHovered
                    pressedColor: JamiTheme.buttonTintedBluePressed

                    onClicked: {
                        createAccount()
                        createAccountStack.currentIndex += 1
                    }
                }

                AccountCreationStepIndicator {
                    Layout.topMargin: backButtonMargins
                    Layout.bottomMargin: backButtonMargins
                    Layout.alignment: Qt.AlignHCenter

                    spacing: layoutSpacing
                    steps: 3
                    currentStep: 2
                }
            }
        }
    }

    PushButton {
        id: backButton

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: backButtonMargins

        width: 35
        height: 35

        normalColor: root.color

        source: "qrc:/images/icons/ic_arrow_back_24px.svg"
        toolTipText: JamiStrings.back

        onClicked: {
            if (createAccountStack.currentIndex == 0)
                leavePage()
            else
                createAccountStack.currentIndex -= 1
        }
    }
}
