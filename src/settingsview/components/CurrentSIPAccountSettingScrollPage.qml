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
    signal navigateToMainView
    signal navigateToNewWizardView

    function updateAccountInfoDisplayed() {
        displaySIPNameLineEdit.text = ClientWrapper.settingsAdaptor.getCurrentAccount_Profile_Info_Alias()
        usernameSIP.text = ClientWrapper.settingsAdaptor.getAccountConfig_Username()
        hostnameSIP.text = ClientWrapper.settingsAdaptor.getAccountConfig_Hostname()
        passSIPlineEdit.text = ClientWrapper.settingsAdaptor.getAccountConfig_Password()
        proxySIP.text = ClientWrapper.settingsAdaptor.getAccountConfig_ProxyServer()

        accountSIPEnableCheckBox.checked = ClientWrapper.settingsAdaptor.get_CurrentAccountInfo_Enabled()

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
                    ClientWrapper.settingsAdaptor.getAvatarImage_Base64(
                        currentSIPAccountAvatar.boothWidht),
                    ClientWrapper.settingsAdaptor.getIsDefaultAvatar())
    }

    function stopBooth() {
        currentSIPAccountAvatar.stopBooth()
    }

    // slots
    function setAccEnableSlot(state) {
        ClientWrapper.accountModel.setAccountEnabled(ClientWrapper.utilsAdaptor.getCurrAccId(), state)
    }

    function delAccountSlot() {
        deleteAccountDialog_SIP.open()
    }

    DeleteAccountDialog{
        id: deleteAccountDialog_SIP

        anchors.centerIn: parent.Center
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        onAccepted: {
            ClientWrapper.accountAdaptor.setSelectedAccountId()
            ClientWrapper.accountAdaptor.setSelectedConvId()

            if(ClientWrapper.utilsAdaptor.getAccountListSize() > 0){
                navigateToMainView()
            } else {
                navigateToNewWizardView()
            }
        }
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true

            Layout.maximumHeight: 10
            Layout.minimumHeight: 10
            Layout.preferredHeight: 10

            Layout.alignment: Qt.AlignTop
        }

        RowLayout {
            spacing: 6

            Layout.alignment: Qt.AlignTop

            Layout.fillWidth: true
            Layout.maximumHeight: 31
            Layout.minimumHeight: 0
            Layout.preferredHeight: accountPageTitleSIP.height

            Item {
                Layout.fillHeight: true

                Layout.maximumWidth: 48
                Layout.preferredWidth: 48
                Layout.minimumWidth: 48
            }

            Label {
                id: accountPageTitleSIP

                Layout.preferredWidth: 133

                Layout.preferredHeight: 31
                Layout.minimumHeight: 25

                text: qsTr("SIP Account")

                font.pointSize: 15
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        ScrollView {
            id: accountSIPScrollView

            property ScrollBar hScrollBar: ScrollBar.horizontal
            property ScrollBar vScrollBar: ScrollBar.vertical

            Layout.fillHeight: true
            Layout.fillWidth: true

            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            font.pointSize: 8
            font.kerning: true
            clip: true

            ColumnLayout {
                id: accountSIPLayout

                Layout.fillHeight: true
                Layout.maximumWidth: 598

                Item {
                    Layout.fillHeight: true

                    Layout.maximumWidth: 30
                    Layout.preferredWidth: 30
                    Layout.minimumWidth: 30
                }

                ColumnLayout {
                    spacing: 6
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Layout.leftMargin: 48

                    Item {
                        Layout.fillHeight: true

                        Layout.maximumWidth: 24
                        Layout.preferredWidth: 24
                        Layout.minimumWidth: 24
                    }

                    ToggleSwitch {
                        id: accountSIPEnableCheckBox

                        labelText: qsTr("Enable")
                        fontPointSize: 10

                        onSwitchToggled: {
                            setAccEnableSlot(checked)
                        }
                    }

                    Item {
                        Layout.fillHeight: true

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true

                            Layout.maximumHeight: 21
                            Layout.preferredHeight: 21
                            Layout.minimumHeight: 21

                            text: qsTr("Profile")
                            font.pointSize: 13
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillWidth: true

                            Layout.maximumHeight: 10
                            Layout.preferredHeight: 10
                            Layout.minimumHeight: 10
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            layoutDirection: Qt.LeftToRight

                            spacing: 6

                            PhotoboothView {
                                id: currentSIPAccountAvatar

                                Layout.alignment: Qt.AlignHCenter

                                Layout.maximumWidth: 261
                                Layout.preferredWidth: 261
                                Layout.minimumWidth: 261
                                Layout.maximumHeight: 261
                                Layout.preferredHeight: 261
                                Layout.minimumHeight: 261

                                Layout.leftMargin: 20

                                onImageAcquired: {
                                    ClientWrapper.settingsAdaptor.setCurrAccAvatar(imgBase64)
                                }

                                onImageCleared: {
                                    ClientWrapper.settingsAdaptor.clearCurrentAvatar()
                                    setAvatar()
                                }
                            }

                            InfoLineEdit {
                                id: displaySIPNameLineEdit

                                fieldLayoutWidth: 261

                                Layout.leftMargin: 20

                                font.pointSize: 10
                                font.kerning: true

                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                onEditingFinished: {
                                    ClientWrapper.accountAdaptor.setCurrAccDisplayName(
                                                displaySIPNameLineEdit.text)
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true

                        Layout.maximumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.minimumHeight: 20
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            Layout.fillWidth: true

                            Layout.maximumHeight: 27
                            Layout.preferredHeight: 27
                            Layout.minimumHeight: 27

                            text: qsTr("Identity")
                            font.pointSize: 13
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillWidth: true

                            Layout.maximumHeight: 10
                            Layout.preferredHeight: 10
                            Layout.minimumHeight: 10
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            GridLayout {
                                rows: 4
                                columns: 2
                                flow: GridLayout.LeftToRight
                                rowSpacing: 14
                                columnSpacing: 6

                                Layout.fillWidth: true

                                Layout.leftMargin: 20

                                // user name
                                Label {
                                    Layout.maximumWidth: 76
                                    Layout.preferredWidth: 76
                                    Layout.minimumWidth: 76

                                    Layout.maximumHeight: 30
                                    Layout.preferredHeight: 30
                                    Layout.minimumHeight: 30

                                    text: qsTr("Username")
                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }

                                InfoLineEdit {
                                    id: usernameSIP

                                    fieldLayoutWidth: 300

                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    onEditingFinished: {
                                        ClientWrapper.settingsAdaptor.setAccountConfig_Username(
                                                    usernameSIP.text)
                                    }
                                }

                                // host name
                                Label {
                                    Layout.maximumWidth: 76
                                    Layout.preferredWidth: 76
                                    Layout.minimumWidth: 76

                                    Layout.maximumHeight: 30
                                    Layout.preferredHeight: 30
                                    Layout.minimumHeight: 30

                                    text: qsTr("Hostname")
                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }

                                InfoLineEdit {
                                    id: hostnameSIP

                                    fieldLayoutWidth: 300

                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    onEditingFinished: {
                                        ClientWrapper.settingsAdaptor.setAccountConfig_Hostname(
                                                    hostnameSIP.text)
                                    }
                                }

                                // proxy
                                Label {
                                    Layout.maximumWidth: 76
                                    Layout.preferredWidth: 76
                                    Layout.minimumWidth: 76

                                    Layout.maximumHeight: 30
                                    Layout.preferredHeight: 30
                                    Layout.minimumHeight: 30

                                    text: qsTr("Proxy")
                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }

                                InfoLineEdit {
                                    id: proxySIP

                                    fieldLayoutWidth: 300

                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    onEditingFinished: {
                                        ClientWrapper.settingsAdaptor.setAccountConfig_ProxyServer(
                                                    proxySIP.text)
                                    }
                                }

                                // password
                                Label {
                                    Layout.maximumWidth: 76
                                    Layout.preferredWidth: 76
                                    Layout.minimumWidth: 76

                                    Layout.maximumHeight: 30
                                    Layout.preferredHeight: 30
                                    Layout.minimumHeight: 30

                                    text: qsTr("Password")
                                    font.pointSize: 10
                                    font.kerning: true

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }

                                InfoLineEdit {
                                    id: passSIPlineEdit

                                    fieldLayoutWidth: 300

                                    font.pointSize: 10
                                    font.kerning: true

                                    echoMode: TextInput.Password
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    onEditingFinished: {
                                        ClientWrapper.settingsAdaptor.setAccountConfig_Password(
                                                    passSIPlineEdit.text)
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true

                                Layout.maximumHeight: 10
                                Layout.preferredHeight: 10
                                Layout.minimumHeight: 10
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.maximumHeight: 30
                                Layout.leftMargin: 20

                                HoverableButtonTextItem {
                                    id: btnSIPDeletAccount

                                    backgroundColor: "red"
                                    onEnterColor: Qt.rgba(150 / 256, 0, 0, 0.7)
                                    onDisabledBackgroundColor: Qt.rgba(
                                                                   255 / 256,
                                                                   0, 0, 0.8)
                                    onPressColor: backgroundColor
                                    textColor: "white"

                                    Layout.maximumWidth: 261
                                    Layout.preferredWidth: 261
                                    Layout.minimumWidth: 261

                                    Layout.maximumHeight: 30
                                    Layout.preferredHeight: 30
                                    Layout.minimumHeight: 30

                                    radius: height / 2

                                    text: qsTr("Delete Account")
                                    font.pointSize: 10
                                    font.kerning: true

                                    onClicked: {
                                        delAccountSlot()
                                    }
                                }

                                Item {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true

                        Layout.maximumHeight: 40
                        Layout.preferredHeight: 40
                        Layout.minimumHeight: 40
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Layout.minimumHeight: 30
                        Layout.preferredHeight: 30
                        Layout.maximumHeight: 30

                        Layout.minimumWidth: 598
                        Layout.preferredWidth: 598

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        HoverableRadiusButton {
                            id: advancedAccountSettingsSIPButton

                            Layout.minimumWidth: 180

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            radius: height / 2

                            text: qsTr("Advanced Account Settings")
                            font.pointSize: 10
                            font.kerning: true

                            icon.source: {
                                if (advanceSIPSettingsView.visible) {
                                    return "qrc:/images/icons/round-arrow_drop_up-24px.svg"
                                } else {
                                    return "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                                }
                            }

                            icon.height: 24
                            icon.width: 24

                            onClicked: {
                                advanceSIPSettingsView.visible = !advanceSIPSettingsView.visible
                                if(advanceSIPSettingsView.visible){
                                    advanceSIPSettingsView.updateAccountInfoDisplayedAdvanceSIP()
                                    var coor = advancedAccountSettingsSIPButton.mapToItem(accountSIPLayout,advancedAccountSettingsSIPButton.x,advancedAccountSettingsSIPButton.y)
                                     accountSIPScrollView.vScrollBar.position  = coor.y / accountSIPLayout.height
                                } else {
                                     accountSIPScrollView.vScrollBar.position = 0
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 48
                    Layout.preferredHeight: 48
                    Layout.maximumHeight: 48
                }

                ColumnLayout {
                    spacing: 6
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Layout.leftMargin: 30

                    // instantiate advance setting page
                    AdvancedSIPSettingsView {
                        id: advanceSIPSettingsView

                        Layout.leftMargin: 10
                        visible: false
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
