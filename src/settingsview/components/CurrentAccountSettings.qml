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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property bool isSIP

    property int contentWidth: currentAccountSettingsColumnLayout.width
    property int preferredHeight: currentAccountSettingsColumnLayout.implicitHeight
    property int preferredColumnWidth : Math.min(root.width / 2 - 50, 350)

    signal navigateToMainView
    signal navigateToNewWizardView
    signal advancedSettingsToggled(bool settingsVisible)

    function updateAccountInfoDisplayed() {
        bannedContacts.updateAndShowBannedContactsSlot()
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
        deleteAccountDialog.open()
    }

    function getAdvancedSettingsScrollPosition() {
        return advancedSettings.y
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

    color: JamiTheme.secondaryBackgroundColor

    SimpleMessageDialog {
        id: msgDialog

        buttonTitles: [qsTr("Ok")]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue]
        buttonCallBacks: [setPasswordButtonText]
    }

    DeleteAccountDialog {
        id: deleteAccountDialog

        onAccepted: {
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
                    var isSuccessful = AccountAdapter.model.exportToFile(LRCInstance.currentAccountId,
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

            checked: CurrentAccount.enabled

            onSwitchToggled: CurrentAccount.enabled = checked
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

            visible: !isSIP && CurrentAccount.managerUri === ""
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: JamiTheme.preferredMarginSize

            preferredWidth: JamiTheme.preferredFieldWidth
            preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            toolTipText: AccountAdapter.hasPassword() ?
                             JamiStrings.changeCurrentPassword :
                             JamiStrings.setAPassword
            text: AccountAdapter.hasPassword() ?
                      JamiStrings.changePassword :
                      JamiStrings.setPassword

            iconSource: JamiResources.round_edit_24dp_svg

            onClicked: passwordClicked()
        }

        MaterialButton {
            id: btnExportAccount

            visible: !isSIP && CurrentAccount.managerUri === ""
            Layout.alignment: Qt.AlignHCenter

            preferredWidth: JamiTheme.preferredFieldWidth
            preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            toolTipText: JamiStrings.tipBackupAccount
            text: JamiStrings.backupAccountBtn

            iconSource: JamiResources.round_save_alt_24dp_svg

            onClicked: exportAccountSlot()
        }

        MaterialButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            preferredWidth: JamiTheme.preferredFieldWidth
            preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedRed
            hoveredColor: JamiTheme.buttonTintedRedHovered
            pressedColor: JamiTheme.buttonTintedRedPressed

            text: JamiStrings.deleteAccount

            iconSource: JamiResources.delete_forever_24dp_svg

            onClicked: delAccountSlot()
        }

        LinkedDevices {
            id: linkedDevices
            visible: !isSIP

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

            onShowAdvancedSettingsRequest: {
                advancedSettingsToggled(settingsVisible)
            }
        }
    }
}
