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
    id: sipAccountViewRect
    signal navigateToMainView
    signal navigateToNewWizardView
    signal backArrowClicked

    property int preferredColumnWidth : sipAccountViewRect.width / 2 - 50

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
                        currentSIPAccountAvatar.boothWidth),
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
    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView
    anchors.centerIn: parent

    ColumnLayout {
        anchors.fill: sipAccountViewRect

        RowLayout {
            id: sipAccountPageTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.fillWidth: true
            Layout.maximumHeight: 64
            Layout.minimumHeight: 64
            Layout.preferredHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.rightMargin: JamiTheme.preferredMarginSize

                radius: 32
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

            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Account Settings")
                fontSize: JamiTheme.titleFontSize
                maxWidth: !backToSettingsMenuButton.visible ? sipAccountViewRect.width - 100 :
                                                              sipAccountViewRect.width - backToSettingsMenuButton.width - 100

            }
        }

        ScrollView {
            id: sipAccountScrollView

            property ScrollBar vScrollBar: ScrollBar.vertical

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            width: sipAccountViewRect.width
            height: sipAccountViewRect.height - sipAccountPageTitle.height

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            clip: true

            ColumnLayout {
                id: accountSIPLayout

                Layout.fillHeight: true
                Layout.preferredWidth: sipAccountViewRect.width
                Layout.alignment: Qt.AlignHCenter

                spacing: 24

                ToggleSwitch {
                    id: accountSIPEnableCheckBox

                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.leftMargin: JamiTheme.preferredMarginSize

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
                    spacing: 8

                    Label {
                        Layout.fillWidth: true

                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("Profile")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    PhotoboothView {
                        id: currentSIPAccountAvatar

                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                        boothWidth: Math.min(224, sipAccountViewRect.width - 100)

                        Layout.maximumWidth: boothWidth+50
                        Layout.preferredWidth: boothWidth+50
                        Layout.minimumWidth: boothWidth+50
                        Layout.maximumHeight: boothWidth+50
                        Layout.preferredHeight: boothWidth+50
                        Layout.minimumHeight: boothWidth+50

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

                        Layout.maximumWidth: JamiTheme.preferredButtonWidth
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        Layout.alignment: Qt.AlignHCenter

                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

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
                    spacing: 8

                    ElidedTextLabel {
                        Layout.fillWidth: true

                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Identity")
                        maxWidth: sipAccountViewRect.width - 72
                        fontSize: JamiTheme.headerFontSize
                    }


                    GridLayout {
                        rows: 4
                        columns: 2
                        flow: GridLayout.LeftToRight
                        rowSpacing: 8
                        columnSpacing: 6

                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        // user name
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Username")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        InfoLineEdit {
                            id: usernameSIP

                            fieldLayoutWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize // Albert: buttonSize?
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            onEditingFinished: {
                                ClientWrapper.settingsAdaptor.setAccountConfig_Username(
                                            usernameSIP.text)
                            }
                        }

                        // host name
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Hostname")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        InfoLineEdit {
                            id: hostnameSIP

                            fieldLayoutWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize // Albert: button?
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            onEditingFinished: {
                                ClientWrapper.settingsAdaptor.setAccountConfig_Hostname(
                                            hostnameSIP.text)
                            }
                        }

                        // proxy
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight

                            text: qsTr("Proxy")
                            font.pointSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        InfoLineEdit {
                            id: proxySIP

                            fieldLayoutWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize // Albert
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            onEditingFinished: {
                                ClientWrapper.settingsAdaptor.setAccountConfig_ProxyServer(
                                            proxySIP.text)
                            }
                        }

                        // password
                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Password")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        InfoLineEdit {
                            id: passSIPlineEdit

                            fieldLayoutWidth: preferredColumnWidth

                            font.pointSize: JamiTheme.settingsFontSize
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


                    HoverableButtonTextItem {
                        id: btnDeletAccount

                        backgroundColor: "red"
                        onEnterColor: Qt.rgba(150 / 256, 0, 0, 0.7)
                        onDisabledBackgroundColor: Qt.rgba(
                                                        255 / 256,
                                                        0, 0, 0.8)
                        onPressColor: backgroundColor
                        textColor: "white"

                        Layout.alignment: Qt.AlignHCenter
                        Layout.minimumWidth: JamiTheme.preferredButtonWidth
                        Layout.preferredWidth: JamiTheme.preferredButtonWidth
                        Layout.maximumWidth: JamiTheme.preferredButtonWidth
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("Delete Account")
                        toolTipText: qsTr("Delete this account")
                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true
                        radius: height / 2

                        onClicked: {
                            delAccountSlot()
                        }
                    }
                }

                RowLayout {
                    id: rowAdvancedSettingsBtn
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {

                        id: lblAdvancedAccountSettings

                        Layout.fillWidth: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Advanced Account Settings")
                        fontSize: JamiTheme.headerFontSize
                        maxWidth: sipAccountViewRect.width - advancedAccountSettingsSIPButton.width - 80
                    }

                    HoverableRadiusButton {
                        id: advancedAccountSettingsSIPButton

                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Layout.minimumWidth: JamiTheme.preferredFieldHeight
                        Layout.preferredWidth: JamiTheme.preferredFieldHeight
                        Layout.maximumWidth: JamiTheme.preferredFieldHeight
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        Layout.alignment: Qt.AlignHCenter

                        radius: height / 2

                        icon.source: {
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
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    visible: false
                }

                Item {
                    Layout.preferredWidth: sipAccountViewRect.width - 32
                    Layout.minimumWidth: sipAccountViewRect.width - 32
                    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView - 32
                    Layout.fillHeight: true
                }
            }
        }
    }
}
