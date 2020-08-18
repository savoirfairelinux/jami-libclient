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

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0

import "../constant"
/*
 * PasswordDialog for changing password and exporting account
 */
Dialog {
    id: passwordDialog

    enum PasswordEnteringPurpose {
        ChangePassword,
        ExportAccount,
        SetPassword
    }
    readonly property int successCode: 200
    signal doneSignal(int code, int currentPurpose)

    property string path: ""
    property int purpose: PasswordDialog.ChangePassword

    header : Rectangle {
        width: parent.width
        height: 64
        color: "transparent"
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 24

            text: {
                switch(purpose){
                case PasswordDialog.ExportAccount:
                    return qsTr("Enter the password of this account")
                case PasswordDialog.ChangePassword:
                    return qsTr("Changing password")
                case PasswordDialog.SetPassword:
                    return qsTr("Set password")
                }
            }
            font.pointSize: JamiTheme.headerFontSize
        }
    }

    function openDialog(purposeIn, exportPathIn = ""){
        purpose = purposeIn
        path = exportPathIn
        currentPasswordEdit.clear()
        passwordEdit.borderColorMode = InfoLineEdit.NORMAL
        confirmPasswordEdit.borderColorMode = InfoLineEdit.NORMAL
        passwordEdit.clear()
        confirmPasswordEdit.clear()
        passwordDialog.open()
    }

    function haveDone(code, currentPurpose) {
        done(code)
        doneSignal(code, currentPurpose)
    }

    function validatePassword() {
        var acceptablePassword =  (passwordEdit.text === confirmPasswordEdit.text)
        btnChangePasswordConfirm.enabled = acceptablePassword

        if (acceptablePassword) {
            passwordEdit.borderColorMode = InfoLineEdit.RIGHT
            confirmPasswordEdit.borderColorMode = InfoLineEdit.RIGHT
            return
        }

        passwordEdit.borderColorMode = InfoLineEdit.ERROR
        confirmPasswordEdit.borderColorMode = InfoLineEdit.ERROR
    }

    Timer{
        id: timerToOperate

        interval: 200
        repeat: false

        onTriggered: {
            if ((purpose === PasswordDialog.ChangePassword) || (purpose === PasswordDialog.SetPassword)) {
                savePasswordQML()
            } else if(purpose === PasswordDialog.ExportAccount) {
                exportAccountQML()
            }
        }
    }

    function exportAccountQML() {
        var success = false
        if(path.length > 0){
            success = ClientWrapper.accountAdaptor.exportToFile(ClientWrapper.utilsAdaptor.getCurrAccId(),path,currentPasswordEdit.text)
        }

        if (success) {
            haveDone(successCode, passwordDialog.purpose)
        } else {
            btnChangePasswordConfirm.enabled = false
            currentPasswordEdit.borderColorMode = InfoLineEdit.ERROR
        }
    }
    function savePasswordQML() {
        var success = false
        success = ClientWrapper.accountAdaptor.savePassword(ClientWrapper.utilsAdaptor.getCurrAccId(),currentPasswordEdit.text, passwordEdit.text)

        if (success) {
            ClientWrapper.accountAdaptor.setArchiveHasPassword(passwordEdit.text.length !== 0)
            haveDone(successCode, passwordDialog.purpose)
        } else {
            currentPasswordEdit.borderColorMode = InfoLineEdit.ERROR
            btnChangePasswordConfirm.enabled = false
        }
    }

    visible: false

    anchors.centerIn: parent.Center
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    height: contentLayout.implicitHeight + 64 + 16

    contentItem: Rectangle {
        implicitWidth: 280

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            spacing: 8

            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true

                spacing: 7

                ColumnLayout {
                    spacing: 0

                    Layout.alignment: Qt.AlignHCenter

                    MaterialLineEdit {
                        id: currentPasswordEdit

                        Layout.maximumHeight: visible ?
                                                48 :
                                                0
                        Layout.fillWidth: true

                        visible: purpose === PasswordDialog.ChangePassword || purpose === PasswordDialog.ExportAccount
                        echoMode: TextInput.Password
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter

                        placeholderText: qsTr("Enter Current Password")

                        onTextChanged: {
                            if (purpose === PasswordDialog.ChangePassword) {
                                validatePassword()
                            }

                            if (currentPasswordEdit.text.length == 0) {
                                btnChangePasswordConfirm.enabled = false
                            } else {
                                btnChangePasswordConfirm.enabled = true
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 8
                    Layout.preferredHeight: 8
                    Layout.maximumHeight: 8
                }

                MaterialLineEdit {
                    id: passwordEdit

                    fieldLayoutHeight: 48
                    layoutFillwidth: true

                    visible: purpose === PasswordDialog.ChangePassword || purpose === PasswordDialog.SetPassword
                    echoMode: TextInput.Password
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    placeholderText: qsTr("Enter New Password")

                    onTextChanged: {
                        validatePassword()
                    }
                }

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 8
                    Layout.preferredHeight: 8
                    Layout.maximumHeight: 8
                }

                MaterialLineEdit {
                    id: confirmPasswordEdit

                    fieldLayoutHeight: 48
                    layoutFillwidth: true

                    visible: purpose === PasswordDialog.ChangePassword || purpose === PasswordDialog.SetPassword
                    echoMode: TextInput.Password
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    placeholderText: qsTr("Confirm New Password")

                    onTextChanged: {
                        validatePassword()
                    }
                }
            }

            RowLayout {
                spacing: 8

                Layout.fillWidth: true

                Layout.alignment: Qt.AlignRight

                Button {
                    id: btnChangePasswordConfirm

                    contentItem: Text {
                        text: qsTr("CONFIRM")
                        color: JamiTheme.buttonTintedBlue
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        timerToOperate.restart()
                    }
                }


                Button {
                    id: btnChangePasswordCancel

                    contentItem: Text {
                        text: qsTr("CANCEL")
                        color: JamiTheme.buttonTintedBlue
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        passwordDialog.reject()
                    }
                }
            }
        }
    }
}
