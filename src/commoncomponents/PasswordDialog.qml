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
import net.jami.Adapters 1.0

import "../constant"

// PasswordDialog for changing password and exporting account

Dialog {
    id: root

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
        id: dialogHeader
        width: parent.width
        height: 64
        color: "transparent"
        Text {
            anchors.fill: parent
            anchors.leftMargin: JamiTheme.preferredMarginSize
            anchors.topMargin: JamiTheme.preferredMarginSize

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
            wrapMode: Text.Wrap
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
        root.open()
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
        if (path.length > 0) {
            success = AccountAdapter.exportToFile(
                        UtilsAdapter.getCurrAccId(),
                        path,
                        currentPasswordEdit.text)
        }

        if (success) {
            haveDone(successCode, root.purpose)
        } else {
            btnChangePasswordConfirm.enabled = false
            currentPasswordEdit.borderColorMode = InfoLineEdit.ERROR
        }
    }

    function savePasswordQML() {
        var success = false
        success = AccountAdapter.savePassword(
                    UtilsAdapter.getCurrAccId(),
                    currentPasswordEdit.text,
                    passwordEdit.text)
        if (success) {
            AccountAdapter.setArchiveHasPassword(passwordEdit.text.length !== 0)
            haveDone(successCode, passwordDialog.purpose)
        } else {
            currentPasswordEdit.borderColorMode = InfoLineEdit.ERROR
            btnChangePasswordConfirm.enabled = false
        }
    }

    visible: false
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    contentItem: Rectangle {
        implicitHeight: contentLayout.implicitHeight + dialogHeader.height + JamiTheme.preferredMarginSize
        implicitWidth: 350

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent

            ColumnLayout {
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

                MaterialLineEdit {
                    id: passwordEdit

                    Layout.fillWidth: true
                    Layout.topMargin: JamiTheme.preferredMarginSize / 2
                    fieldLayoutHeight: 48

                    visible: purpose === PasswordDialog.ChangePassword || purpose === PasswordDialog.SetPassword
                    echoMode: TextInput.Password
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    placeholderText: qsTr("Enter New Password")

                    onTextChanged: {
                        validatePassword()
                    }
                }

                MaterialLineEdit {
                    id: confirmPasswordEdit

                    Layout.fillWidth: true
                    Layout.topMargin: JamiTheme.preferredMarginSize / 2
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
                Layout.topMargin: JamiTheme.preferredMarginSize / 2
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
                    Layout.leftMargin: JamiTheme.preferredMarginSize / 2

                    contentItem: Text {
                        text: qsTr("CANCEL")
                        color: JamiTheme.buttonTintedBlue
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        root.reject()
                    }
                }
            }
        }
    }
}
