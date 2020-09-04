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
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: root
    signal navigateToMainView
    signal navigateToNewWizardView
    signal backArrowClicked

    property int preferredColumnWidth : root.width / 2 - 50

    function updateAccountInfoDisplayed() {
        displaySIPNameLineEdit.text = SettingsAdapter.getCurrentAccount_Profile_Info_Alias()
        usernameSIP.text = SettingsAdapter.getAccountConfig_Username()
        hostnameSIP.text = SettingsAdapter.getAccountConfig_Hostname()
        passSIPlineEdit.text = SettingsAdapter.getAccountConfig_Password()
        proxySIP.text = SettingsAdapter.getAccountConfig_ProxyServer()

        accountSIPEnableCheckBox.checked = SettingsAdapter.get_CurrentAccountInfo_Enabled()

        setAvatar()

        if (advanceSIPSettingsView.visible) {
            advanceSIPSettingsView.updateAccountInfoDisplayedAdvanceSIP()
        }
    }

    function isPhotoBoothOpened() {
        return currentSIPAccountAvatar.takePhotoState
    }

    function setAvatar() {
        currentSIPAccountAvatar.setAvatarPixmap(
                   SettingsAdapter.getAvatarImage_Base64(
                        currentSIPAccountAvatar.boothWidth),
                   SettingsAdapter.getIsDefaultAvatar())
    }

    function stopBooth() {
        currentSIPAccountAvatar.stopBooth()
    }

    // slots
    function setAccEnableSlot(state) {
        ClientWrapper.accountModel.setAccountEnabled(UtilsAdapter.getCurrAccId(), state)
    }

    function delAccountSlot() {
        deleteAccountDialog_SIP.open()
    }

    DeleteAccountDialog {
        id: deleteAccountDialog_SIP

        anchors.centerIn: parent.Center

        onAccepted: {
            ClientWrapper.accountAdaptor.setSelectedConvId()

            if(UtilsAdapter.getAccountListSize() > 0){
                navigateToMainView()
            } else {
                navigateToNewWizardView()
            }
        }
    }

    ColumnLayout {
        anchors.fill: root

        RowLayout {
            id: sipAccountPageTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.minimumHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.preferredWidth: JamiTheme.preferredFieldHeight

                radius: JamiTheme.preferredFieldHeight
                source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                backgroundColor: "white"
                onExitColor: "white"
                toolTipText: qsTr("Toggle to display side panel")
                hoverEnabled: true
                visible: mainViewWindow.sidePanelHidden

                onClicked: {
                    backArrowClicked()
                }
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("Account Settings")

                font.pointSize: JamiTheme.titleFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }

        ScrollView {
            id: sipAccountScrollView

            property ScrollBar vScrollBar: ScrollBar.vertical

            Layout.fillHeight: true
            Layout.fillWidth: true

            clip: true

            ColumnLayout {
                id: accountSIPLayout

                width: root.width

                ToggleSwitch {
                    id: accountSIPEnableCheckBox

                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    labelText: qsTr("Enable")
                    fontPointSize: JamiTheme.headerFontSize

                    onSwitchToggled: {
                        setAccEnableSlot(checked)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    Label {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("Profile")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    PhotoboothView {
                        id: currentSIPAccountAvatar

                        Layout.alignment: Qt.AlignCenter

                        boothWidth: Math.min(224, root.width - 100) + 50

                        Layout.preferredWidth: boothWidth

                        onImageAcquired: {
                           SettingsAdapter.setCurrAccAvatar(imgBase64)
                        }

                        onImageCleared: {
                           SettingsAdapter.clearCurrentAvatar()
                            setAvatar()
                        }
                    }

                    MaterialLineEdit {
                        id: displaySIPNameLineEdit

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth

                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        padding: 8

                        onEditingFinished: {
                            ClientWrapper.accountAdaptor.setCurrAccDisplayName(
                                        displaySIPNameLineEdit.text)
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Identity")
                        maxWidth: root.width - 72
                        fontSize: JamiTheme.headerFontSize
                    }


                    GridLayout {
                        rows: 4
                        columns: 2
                        flow: GridLayout.LeftToRight

                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        // user name
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Username")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        MaterialLineEdit {
                            id: usernameSIP

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            padding: 8

                            onEditingFinished: {
                               SettingsAdapter.setAccountConfig_Username(
                                            usernameSIP.text)
                            }
                        }

                        // host name
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Hostname")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        MaterialLineEdit {
                            id: hostnameSIP

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            padding: 8

                            onEditingFinished: {
                               SettingsAdapter.setAccountConfig_Hostname(
                                            hostnameSIP.text)
                            }
                        }

                        // proxy
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: qsTr("Proxy")
                            font.pointSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        MaterialLineEdit {
                            id: proxySIP

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            padding: 8

                            onEditingFinished: {
                               SettingsAdapter.setAccountConfig_ProxyServer(
                                            proxySIP.text)
                            }
                        }

                        // password
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Password")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        MaterialLineEdit {
                            id: passSIPlineEdit

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            echoMode: TextInput.Password
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            padding: 8

                            onEditingFinished: {
                               SettingsAdapter.setAccountConfig_Password(
                                            passSIPlineEdit.text)
                            }
                        }
                    }


                    MaterialButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        color: JamiTheme.buttonTintedRed
                        hoveredColor: JamiTheme.buttonTintedRedHovered
                        pressedColor: JamiTheme.buttonTintedRedPressed

                        toolTipText: qsTr("Press this button to delete this account")
                        text: qsTr("Delete Account")

                        source: "qrc:/images/icons/delete_forever-24px.svg"

                        onClicked: {
                            delAccountSlot()
                        }
                    }
                }

                RowLayout {
                    id: rowAdvancedSettingsBtn
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: 8

                    ElidedTextLabel {
                        id: lblAdvancedAccountSettings

                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Advanced Account Settings")
                        fontSize: JamiTheme.headerFontSize
                        maxWidth: root.width - advancedAccountSettingsSIPButton.width
                                  - JamiTheme.preferredMarginSize * 6
                    }

                    HoverableButtonTextItem {
                        id: advancedAccountSettingsSIPButton

                        Layout.preferredWidth: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.alignment: Qt.AlignHCenter

                        radius: height / 2

                        toolTipText: qsTr("Press to display or hide advance settings")

                        source: {
                            if (advanceSIPSettingsView.visible) {
                                return "qrc:/images/icons/round-arrow_drop_up-24px.svg"
                            } else {
                                return "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                            }
                        }

                        onClicked: {
                            advanceSIPSettingsView.visible = !advanceSIPSettingsView.visible
                            if(advanceSIPSettingsView.visible){
                                advanceSIPSettingsView.updateAccountInfoDisplayedAdvanceSIP()
                                sipAccountScrollView.vScrollBar.position  = rowAdvancedSettingsBtn.y / accountSIPLayout.height
                            } else {
                                sipAccountScrollView.vScrollBar.position = 0
                            }
                        }
                    }
                }

                // instantiate advance setting page
                AdvancedSIPSettingsView {
                    id: advanceSIPSettingsView
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize
                    visible: false
                    itemWidth: preferredColumnWidth
                }
            }
        }
    }
}
