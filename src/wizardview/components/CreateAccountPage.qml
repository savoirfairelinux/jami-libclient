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
    property int nameRegistrationUIState: WizardView.BLANK
    property alias text_passwordEditAlias: passwordEdit.text

    signal createAccount
    signal leavePage

    function initializeOnShowUp() {
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
                    text: qsTr("Choose a username")
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

            MaterialLineEdit {
                id: usernameEdit

                Layout.topMargin: 15
                Layout.preferredHeight: fieldLayoutHeight
                Layout.preferredWidth: fieldLayoutWidth
                Layout.alignment: Qt.AlignHCenter

                selectByMouse: true
                placeholderText: qsTr("Choose your username")
                font.pointSize: 9
                font.kerning: true

                borderColorMode: {
                    if (nameRegistrationUIState === WizardView.BLANK)
                        return MaterialLineEdit.NORMAL
                    else
                        return nameRegistrationUIState >= WizardView.FREE ?
                                    MaterialLineEdit.NORMAL : MaterialLineEdit.ERROR
                }

                fieldLayoutWidth: chooseUsernameButton.width
            }

            Label {
                Layout.alignment: Qt.AlignHCenter

                visible: text.length !==0

                text: {
                    switch(nameRegistrationUIState){
                    case WizardView.BLANK:
                    case WizardView.SEARCHING:
                    case WizardView.FREE:
                        return ""
                    case WizardView.INVALID:
                        return qsTr("Invalid username")
                    case WizardView.TAKEN:
                        return qsTr("Username already taken")
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

                text: qsTr("CHOOSE USERNAME")
                color: nameRegistrationUIState === WizardView.FREE?
                        JamiTheme.buttonTintedGrey
                        : JamiTheme.buttonTintedGreyInactive
                hoveredColor: JamiTheme.buttonTintedGreyHovered
                pressedColor: JamiTheme.buttonTintedGreyPressed

                onClicked: {
                    if (nameRegistrationUIState === WizardView.FREE)
                        createAccountStack.currentIndex = createAccountStack.currentIndex + 1
                }
            }

            Row {
                id: skipAndBackButtonsRow

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: chooseUsernameButton.preferredWidth
                Layout.preferredHeight: chooseUsernameButton.preferredHeight

                spacing: layoutSpacing

                Repeater {
                    model: 2

                    MaterialButton {
                        width: (skipAndBackButtonsRow.width -
                                skipAndBackButtonsRow.spacing) / 2
                        height: skipAndBackButtonsRow.height

                        text: modelData === 0 ? qsTr("BACK") : qsTr("SKIP")
                        color: JamiTheme.buttonTintedGrey
                        hoveredColor: JamiTheme.buttonTintedGreyHovered
                        pressedColor: JamiTheme.buttonTintedGreyPressed
                        outlined: true

                        onClicked: {
                            if (modelData === 0)
                                leavePage()
                            else
                                createAccountStack.currentIndex =
                                        createAccountStack.currentIndex + 1
                        }
                    }
                }
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

                text: qsTr("CREATE ACCOUNT")
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

            MaterialButton {
                id: backButton

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: preferredWidth
                Layout.preferredHeight: preferredHeight

                text: qsTr("BACK")
                color: JamiTheme.buttonTintedGrey
                hoveredColor: JamiTheme.buttonTintedGreyHovered
                pressedColor: JamiTheme.buttonTintedGreyPressed
                outlined: true

                onClicked: createAccountStack.currentIndex -= 1
            }
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
