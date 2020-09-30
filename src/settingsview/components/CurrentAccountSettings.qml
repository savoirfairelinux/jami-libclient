/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.3
import Qt.labs.platform 1.1
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"
import "../../constant"

Rectangle {
    id: root

    property bool isSIP

    property int contentWidth: currentAccountSettingsColumnLayout.width
    property int preferredHeight: currentAccountSettingsColumnLayout.implicitHeight
    property int preferredColumnWidth : Math.min(root.width / 2 - 50, 350)

    signal navigateToMainView
    signal navigateToNewWizardView
    signal advancedSettingsToggled(bool settingsVisible)

    function isPhotoBoothOpened() {
        return accountProfile.isPhotoBoothOpened()
    }

    function updateAccountInfoDisplayed() {
        accountProfile.setAvatar()

        accountEnableCheckBox.checked = SettingsAdapter.get_CurrentAccountInfo_Enabled()
        accountProfile.updateAccountInfo()
        userIdentity.updateAccountInfo()
        linkedDevices.updateAndShowDevicesSlot()
        bannedContacts.setVisibility()
        advancedSettings.updateAdvancedAccountInfos()
        setPasswordButtonText()
    }

    function passwordClicked() {
        if (AccountAdapter.hasPassword()) {
            passwordDialog.openDialog(PasswordDialog.ChangePassword)
        } else {
            passwordDialog.openDialog(PasswordDialog.SetPassword)
        }
    }

    function exportAccountSlot() {
        exportBtn_Dialog.open()
    }

    function delAccountSlot() {
        deleteAccountDialog.openDialog()
    }

    function connectCurrentAccount() {
        if (!isSIP) {
            linkedDevices.connectCurrentAccount(true)
            bannedContacts.connectCurrentAccount(true)
        }
    }

    function disconnectAccountConnections() {
        linkedDevices.connectCurrentAccount(false)
        bannedContacts.connectCurrentAccount(false)
    }

    function getAdvancedSettingsScrollPosition() {
        return advancedSettings.y / currentAccountSettingsColumnLayout.height
    }

    function setPasswordButtonText() {
        var hasPassword = AccountAdapter.hasPassword()
        passwdPushButton.toolTipText = hasPassword ?
                    JamiStrings.changeCurrentPassword :
                    JamiStrings.setAPassword

        passwdPushButton.text = hasPassword ?
                    JamiStrings.changePassword :
                    JamiStrings.setPassword
    }

    SimpleMessageDialog {
        id: msgDialog

        buttonTitles: [qsTr("Ok")]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue]
        buttonCallBacks: [setPasswordButtonText]
    }

    DeleteAccountDialog {
        id: deleteAccountDialog

        onAccepted: {
            AccountAdapter.setSelectedConvId()

            if(UtilsAdapter.getAccountListSize() > 0) {
                navigateToMainView()
            } else {
                navigateToNewWizardView()
            }
        }
    }

    PasswordDialog {
        id: passwordDialog

        onDoneSignal: {
            var title = success ? qsTr("Success") : qsTr("Error")

            var info
            switch(currentPurpose) {
                case PasswordDialog.ExportAccount:
                    info = success ? JamiStrings.backupSuccessful : JamiStrings.backupFailed
                    break
                case PasswordDialog.ChangePassword:
                    info = success ? JamiStrings.changePasswordSuccess : JamiStrings.changePasswordFailed
                    break
                case PasswordDialog.SetPassword:
                    info = success ? JamiStrings.setPasswordSuccess : JamiStrings.setPasswordFailed
                    passwdPushButton.text = success ? JamiStrings.changePassword : JamiStrings.setPassword
                    break
            }

            msgDialog.openWithParameters(title, info)
        }
    }

    JamiFileDialog {
        id: exportBtn_Dialog

        mode: JamiFileDialog.SaveFile

        title: JamiStrings.backupAccountHere
        folder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            // is there password? If so, go to password dialog, else, go to following directly
            var exportPath = UtilsAdapter.getAbsPath(file.toString())
            if (AccountAdapter.hasPassword()) {
                passwordDialog.openDialog(PasswordDialog.ExportAccount,exportPath)
                return
            } else {
                if (exportPath.length > 0) {
                    var isSuccessful = AccountAdapter.model.exportToFile(AccountAdapter.currentAccountId,
                                                                         exportPath, "")
                    var title = isSuccessful ? qsTr("Success") : qsTr("Error")
                    var info = isSuccessful ? JamiStrings.backupSuccessful : JamiStrings.backupFailed

                    msgDialog.openWithParameters(title,info)
                }
            }
        }
    }

    ColumnLayout {
        id: currentAccountSettingsColumnLayout

        anchors.horizontalCenter: root.horizontalCenter

        width: Math.min(JamiTheme.maximumWidthSettingsView, root.width)

        ToggleSwitch {
            id: accountEnableCheckBox

            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            labelText: JamiStrings.enableAccount
            fontPointSize: JamiTheme.headerFontSize

            onSwitchToggled: AccountAdapter.model.setAccountEnabled(
                                 AccountAdapter.currentAccountId, checked)
        }

        AccountProfile {
            id: accountProfile

            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
        }

        UserIdentity {
            id: userIdentity
            isSIP: root.isSIP

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            itemWidth: preferredColumnWidth
        }

        MaterialButton {
            id: passwdPushButton

            visible: !isSIP && SettingsAdapter.getAccountConfig_Manageruri() === ""
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.preferredFieldWidth
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            toolTipText: AccountAdapter.hasPassword() ?
                        JamiStrings.changeCurrentPassword : JamiStrings.setAPassword
            text: AccountAdapter.hasPassword() ? JamiStrings.changePassword : JamiStrings.setPassword

            source: "qrc:/images/icons/round-edit-24px.svg"

            onClicked: {
                passwordClicked()
            }
        }

        MaterialButton {
            id: btnExportAccount

            visible: !isSIP && SettingsAdapter.getAccountConfig_Manageruri() === ""
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.preferredFieldWidth
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            toolTipText: JamiStrings.tipBackupAccount
            text: JamiStrings.backupAccountBtn

            source: "qrc:/images/icons/round-save_alt-24px.svg"

            onClicked: {
                exportAccountSlot()
            }
        }

        MaterialButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.preferredFieldWidth
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.topMargin: isSIP ? JamiTheme.preferredMarginSize : 0
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            color: JamiTheme.buttonTintedRed
            hoveredColor: JamiTheme.buttonTintedRedHovered
            pressedColor: JamiTheme.buttonTintedRedPressed

            text: JamiStrings.deleteAccount

            source: "qrc:/images/icons/delete_forever-24px.svg"

            onClicked: {
                delAccountSlot()
            }
        }

        LinkedDevices {
            id: linkedDevices
            visible: !isSIP && SettingsAdapter.getAccountConfig_Manageruri() === ""

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
        }

        BannedContacts {
            id: bannedContacts
            isSIP: root.isSIP

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
        }

        AdvancedSettings {
            id: advancedSettings

            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
            Layout.bottomMargin: 8

            itemWidth: preferredColumnWidth
            isSIP: root.isSIP

            onHeightChanged: advancedSettingsToggled(settingsVisible)
        }
    }
}
