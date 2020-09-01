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
import QtQuick.Layouts 1.3
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

        title: qsTr("Export Account Here")
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

        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter

        ColumnLayout {
            spacing: layoutSpacing

            Layout.preferredWidth: root.width
            Layout.alignment: Qt.AlignCenter

            RowLayout {
                spacing: layoutSpacing

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: usernameEdit.width

                Label {
                    text: isRendezVous ? qsTr("Choose a name for your rendez-vous") : qsTr("Choose a username for your account")
                    font.pointSize: JamiTheme.textFontSize + 3
                }

                Label {
                    Layout.alignment: Qt.AlignRight

                    text: qsTr("Recommended")
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
                text: isRendezVous ? qsTr("Choose name") : qsTr("Choose username")
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

                text: qsTr("SKIP")
                color: JamiTheme.buttonTintedGrey
                hoveredColor: JamiTheme.buttonTintedGreyHovered
                pressedColor: JamiTheme.buttonTintedGreyPressed
                outlined: true

                onClicked: createAccountStack.currentIndex =
                           createAccountStack.currentIndex + 1
            }
        }

        ColumnLayout {
            spacing: layoutSpacing

            Layout.preferredWidth: root.width
            Layout.alignment: Qt.AlignCenter

            RowLayout {
                spacing: layoutSpacing

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: usernameEdit.width

                Label {
                    text: qsTr("Create a password")
                    font.pointSize: JamiTheme.textFontSize + 3

                    Switch {
                        id: passwordSwitch

                        anchors.left: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignRight

                    text: qsTr("Optional")
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
                placeholderText: qsTr("Password")
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
                placeholderText: qsTr("Confirm password")
                font.pointSize: 9
                font.kerning: true
            }

            Label {
                Layout.alignment: Qt.AlignLeft
                Layout.topMargin: 10
                Layout.leftMargin: (root.width - createAccountButton.width) / 2

                text: qsTr("Note that the password cannot be recovered")
                font.pointSize: JamiTheme.textFontSize
            }

            MaterialButton {
                id: createAccountButton

                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 10
                Layout.preferredWidth: preferredWidth
                Layout.preferredHeight: preferredHeight

                function checkEnable() {
                    return !passwordSwitch.checked ||
                            (passwordEdit.text === passwordConfirmEdit.text
                             && passwordEdit.text.length !== 0)
                }

                fontCapitalization: Font.AllUppercase
                text: isRendezVous ? qsTr("Create rendez-vous") : qsTr("Create account")
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
        }
    }

    HoverableButton {
        id: backButton

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        width: 35
        height: 35

        radius: 30

        backgroundColor: root.color
        onExitColor: root.color

        source: "qrc:/images/icons/ic_arrow_back_24px.svg"
        toolTipText: qsTr("Back")

        onClicked: {
            if (createAccountStack.currentIndex == 0)
                leavePage()
            else
                createAccountStack.currentIndex -= 1
        }
    }

    AccountCreationStepIndicator {
        anchors.bottom: root.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: root.horizontalCenter

        spacing: layoutSpacing
        steps: 3
        currentStep: usernameEdit.visible ? 1 : 2
    }
}
