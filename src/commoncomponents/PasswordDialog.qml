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

import QtQuick
import QtQuick.Layouts

import net.jami.Adapters 1.1
import net.jami.Constants 1.1

// PasswordDialog for changing password and exporting account
BaseDialog {
    id: root

    enum PasswordEnteringPurpose {
        ChangePassword,
        ExportAccount,
        SetPassword
    }

    property string path: ""
    property int purpose: PasswordDialog.ChangePassword

    signal doneSignal(bool success, int currentPurpose)

    function openDialog(purposeIn, exportPathIn = "") {
        purpose = purposeIn
        path = exportPathIn
        currentPasswordEdit.clear()
        passwordEdit.clear()
        confirmPasswordEdit.clear()
        validatePassword()
        open()
    }

    function validatePassword() {
        switch (purpose) {
        case PasswordDialog.ExportAccount:
            btnConfirm.enabled = currentPasswordEdit.length > 0
            break
        case PasswordDialog.SetPassword:
            btnConfirm.enabled = passwordEdit.length > 0 &&
                    passwordEdit.text === confirmPasswordEdit.text
            break
        default:
            btnConfirm.enabled = currentPasswordEdit.length > 0 &&
                    passwordEdit.text === confirmPasswordEdit.text
        }
    }

    function exportAccountQML() {
        var success = false
        if (path.length > 0) {
            success = AccountAdapter.exportToFile(
                        LRCInstance.currentAccountId,
                        path,
                        currentPasswordEdit.text)
        }
        doneSignal(success, purpose)
        close()
    }

    function savePasswordQML() {
        var success = false
        success = AccountAdapter.savePassword(
                    LRCInstance.currentAccountId,
                    currentPasswordEdit.text,
                    passwordEdit.text)
        if (success) {
            AccountAdapter.setArchiveHasPassword(passwordEdit.text.length !== 0)
        }
        doneSignal(success, purpose)
        close()
    }

    title: {
        switch(purpose){
        case PasswordDialog.ExportAccount:
            return JamiStrings.enterPassword
        case PasswordDialog.ChangePassword:
            return JamiStrings.changePassword
        case PasswordDialog.SetPassword:
            return JamiStrings.setPassword
        }
    }

    Timer {
        id: timerToOperate

        interval: 200
        repeat: false

        onTriggered: {
            if (purpose === PasswordDialog.ExportAccount) {
                exportAccountQML()
            } else {
                savePasswordQML()
            }
        }
    }

    contentItem: Rectangle {
        id: passwordDialogContentRect

        implicitWidth: JamiTheme.preferredDialogWidth
        implicitHeight: JamiTheme.preferredDialogHeight
        color: JamiTheme.secondaryBackgroundColor

        ColumnLayout {
            anchors.centerIn: parent
            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize

            MaterialLineEdit {
                id: currentPasswordEdit

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: JamiTheme.preferredFieldWidth
                Layout.preferredHeight: visible ? 48 : 0

                visible: purpose === PasswordDialog.ChangePassword ||
                         purpose === PasswordDialog.ExportAccount
                echoMode: TextInput.Password
                placeholderText: JamiStrings.enterCurrentPassword

                onTextChanged: {
                    validatePassword()
                }
            }

            MaterialLineEdit {
                id: passwordEdit

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: JamiTheme.preferredFieldWidth
                Layout.preferredHeight: visible ? 48 : 0

                visible: purpose === PasswordDialog.ChangePassword ||
                         purpose === PasswordDialog.SetPassword
                echoMode: TextInput.Password

                placeholderText: JamiStrings.enterNewPassword

                onTextChanged: {
                    validatePassword()
                }
            }

            MaterialLineEdit {
                id: confirmPasswordEdit

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: JamiTheme.preferredFieldWidth
                Layout.preferredHeight: visible ? 48 : 0

                visible: purpose === PasswordDialog.ChangePassword ||
                         purpose === PasswordDialog.SetPassword
                echoMode: TextInput.Password

                placeholderText: JamiStrings.confirmNewPassword

                onTextChanged: {
                    validatePassword()
                }
            }

            RowLayout {
                spacing: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter

                MaterialButton {
                    id: btnConfirm

                    Layout.alignment: Qt.AlignHCenter

                    preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8

                    color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    outlined: true
                    enabled: purpose === PasswordDialog.SetPassword

                    text: (purpose === PasswordDialog.ExportAccount) ? JamiStrings.exportAccount :
                                                                      JamiStrings.change

                    onClicked: {
                        btnConfirm.enabled = false
                        timerToOperate.restart()
                    }
                }

                MaterialButton {
                    id: btnCancel

                    Layout.alignment: Qt.AlignHCenter

                    preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    outlined: true

                    text: qsTr("Cancel")

                    onClicked: {
                        close()
                    }
                }
            }
        }
    }
}
