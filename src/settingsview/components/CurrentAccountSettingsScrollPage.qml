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

Rectangle {
    id: root

    property string registeredName: ""
    property bool registeredIdNeedsSet: false

    property int preferredColumnWidth : root.width / 2 - 50

    signal navigateToMainView
    signal navigateToNewWizardView
    signal backArrowClicked

    function updateAccountInfoDisplayed() {
        setAvatar()

        accountEnableCheckBox.checked = SettingsAdapter.get_CurrentAccountInfo_Enabled()
        displayNameLineEdit.text = SettingsAdapter.getCurrentAccount_Profile_Info_Alias()

        var showLocalAccountConfig = (ClientWrapper.SettingsAdapter.getAccountConfig_Manageruri() === "")
        passwdPushButton.visible = showLocalAccountConfig
        btnExportAccount.visible = showLocalAccountConfig
        linkDevPushButton.visible = showLocalAccountConfig

        registeredIdNeedsSet = (ClientWrapper.SettingsAdapter.get_CurrentAccountInfo_RegisteredName() === "")

        if(!registeredIdNeedsSet){
            currentRegisteredID.text = SettingsAdapter.get_CurrentAccountInfo_RegisteredName()
        } else {
            currentRegisteredID.text = ""
        }

        currentRingIDText.text = SettingsAdapter.getCurrentAccount_Profile_Info_Uri()

        // update device list view
        updateAndShowDevicesSlot()

        bannedContactsLayoutWidget.visible = (bannedListWidget.model.rowCount() > 0)

        if (advanceSettingsView.visible) {
            advanceSettingsView.updateAccountInfoDisplayedAdvance()
        }
    }

    function connectCurrentAccount() {
        accountConnections_ContactModel.enabled = true
        accountConnections_DeviceModel.enabled = true
    }

    function disconnectAccountConnections() {
        accountConnections_ContactModel.enabled = false
        accountConnections_DeviceModel.enabled = false
    }

    function isPhotoBoothOpened() {
        return currentAccountAvatar.takePhotoState
    }

    function setAvatar() {
        currentAccountAvatar.setAvatarPixmap(
                   SettingsAdapter.getAvatarImage_Base64(
                        currentAccountAvatar.boothWidth),
                   SettingsAdapter.getIsDefaultAvatar())
    }

    function stopBooth() {
        currentAccountAvatar.stopBooth()
    }

    function toggleBannedContacts() {
        var bannedContactsVisible = bannedContactsListWidget.visible
        bannedContactsListWidget.visible = !bannedContactsVisible
        updateAndShowBannedContactsSlot()
    }

    function unban(index) {
       SettingsAdapter.unbanContact(index)
        updateAndShowBannedContactsSlot()
    }

    Connections {
        id: accountConnections_ContactModel
        target: ClientWrapper.contactModel
        enabled: root.visible

        function onModelUpdated(uri, needsSorted) {
            updateAndShowBannedContactsSlot()
        }

        function onContactAdded(contactUri){
            updateAndShowBannedContactsSlot()
        }

        function onContactRemoved(contactUri){
            updateAndShowBannedContactsSlot()
        }
    }

    Connections {
        id: accountConnections_DeviceModel
        target: ClientWrapper.deviceModel
        enabled: root.visible

        function onDeviceAdded(id) {
            updateAndShowDevicesSlot()
        }

        function onDeviceRevoked(id, status) {
            updateAndShowDevicesSlot()
        }

        function onDeviceUpdated(id) {
            updateAndShowDevicesSlot()
        }
    }

    function setAccEnableSlot(state) {
        ClientWrapper.accountModel.setAccountEnabled(ClientWrapper.utilsAdaptor.getCurrAccId(), state)
    }

    // JamiFileDialog for exporting account
    JamiFileDialog {
        id: exportBtn_Dialog

        mode: JamiFileDialog.SaveFile

        title: qsTr("Export Account Here")
        folder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            // is there password? If so, go to password dialog, else, go to following directly
            var exportPath = ClientWrapper.utilsAdaptor.getAbsPath(file.toString())
            if (ClientWrapper.accountAdaptor.hasPassword()) {
                passwordDialog.openDialog(PasswordDialog.ExportAccount,exportPath)
                return
            } else {
                if (exportPath.length > 0) {
                    var isSuccessful = ClientWrapper.accountModel.exportToFile(ClientWrapper.utilsAdaptor.getCurrAccId(), exportPath,"")
                    var title = isSuccessful ? qsTr("Success") : qsTr("Error")
                    var iconMode = isSuccessful ? StandardIcon.Information : StandardIcon.Critical
                    var info = isSuccessful ? qsTr("Export Successful") : qsTr("Export Failed")
                    MessageBox.openWithParameters(title,info, iconMode, StandardButton.Ok)
                }
            }
        }
    }

    function exportAccountSlot() {
        exportBtn_Dialog.open()
    }

    PasswordDialog {
        id: passwordDialog

        anchors.centerIn: parent.Center

        onDoneSignal: {
            var success = (code === successCode)
            var title = success ? qsTr("Success") : qsTr("Error")
            var iconMode = success ? StandardIcon.Information : StandardIcon.Critical

            var info
            switch(currentPurpose){
            case PasswordDialog.ExportAccount:
                info = success ? qsTr("Export Successful") : qsTr("Export Failed")
                break
            case PasswordDialog.ChangePassword:
                info = success ? qsTr("Password Changed Successfully") : qsTr("Password Change Failed")
                break
            case PasswordDialog.SetPassword:
                info = success ? qsTr("Password Set Successfully") : qsTr("Password Set Failed")
                passwdPushButton.text = success ? qsTr("Change Password") : qsTr("Set Password")
                break
            }

            MessageBox.openWithParameters(title,info, iconMode, StandardButton.Ok)
        }
    }

    function passwordClicked() {
        if (ClientWrapper.accountAdaptor.hasPassword()){
            passwordDialog.openDialog(PasswordDialog.ChangePassword)
        } else {
            passwordDialog.openDialog(PasswordDialog.SetPassword)
        }
    }

    function delAccountSlot() {
        deleteAccountDialog.open()
    }

    DeleteAccountDialog {
        id: deleteAccountDialog

        anchors.centerIn: parent.Center

        onAccepted: {
            ClientWrapper.accountAdaptor.setSelectedConvId()

            if(ClientWrapper.utilsAdaptor.getAccountListSize() > 0){
                navigateToMainView()
            } else {
                navigateToNewWizardView()
            }
        }
    }

    NameRegistrationDialog{
        id : nameRegistrationDialog

        onAccepted: {
            registeredIdNeedsSet = false
            currentRegisteredID.nameRegistrationState =
                    UsernameLineEdit.NameRegistrationState.BLANK
        }
    }

    LinkDeviceDialog{
        id: linkDeviceDialog

        anchors.centerIn: parent.Center

        onAccepted: {
            updateAndShowDevicesSlot()
        }
    }

    function showLinkDevSlot() {
        linkDeviceDialog.openLinkDeviceDialog()
    }

    RevokeDevicePasswordDialog{
        id: revokeDevicePasswordDialog

        anchors.centerIn: parent.Center

        onRevokeDeviceWithPassword:{
            revokeDeviceWithIDAndPassword(idOfDevice, password)
        }
    }

    MessageBox{
        id: revokeDeviceMessageBox

        property string idOfDev: ""

        title:qsTr("Remove Device")
        text :qsTr("Are you sure you wish to remove this device?")
        icon :StandardIcon.Information
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onAccepted: {
            revokeDeviceWithIDAndPassword(idOfDev,"")
        }
    }

    function removeDeviceSlot(index){
        var idOfDevice = settingsListView.model.data(settingsListView.model.index(index,0), DeviceItemListModel.DeviceID)
        if(ClientWrapper.accountAdaptor.hasPassword()){
            revokeDevicePasswordDialog.openRevokeDeviceDialog(idOfDevice)
        } else {
            revokeDeviceMessageBox.idOfDev = idOfDevice
            revokeDeviceMessageBox.open()
        }
    }

    function revokeDeviceWithIDAndPassword(idDevice, password){
        ClientWrapper.deviceModel.revokeDevice(idDevice, password)
        updateAndShowDevicesSlot()
    }

    function updateAndShowBannedContactsSlot() {
        if(bannedListWidget.model.rowCount() <= 0){
            bannedContactsLayoutWidget.visible = false
            return
        }

        bannedListWidget.model.reset()
    }

    function updateAndShowDevicesSlot() {
        if(ClientWrapper.SettingsAdapter.getAccountConfig_Manageruri() === ""){
            linkDevPushButton.visible = true
        }

        settingsListView.model.reset()
    }

    ColumnLayout {
        anchors.fill: root

        RowLayout {
            id: accountPageTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.fillWidth: true
            Layout.preferredHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

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
            id: accountScrollView

            property ScrollBar vScrollBar: ScrollBar.vertical
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Layout.fillHeight: true
            Layout.fillWidth: true

            focus: true
            clip: true

            // ScrollView Layout
            ColumnLayout {
                id: accountViewLayout

                width: root.width

                ToggleSwitch {
                    id: accountEnableCheckBox

                    Layout.fillWidth: true
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    labelText: qsTr("Enable")
                    fontPointSize: JamiTheme.headerFontSize

                    onSwitchToggled: {
                        setAccEnableSlot(checked)
                    }
                }

                // Profile
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: JamiTheme.preferredMarginSize
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
                        id: currentAccountAvatar

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
                        id: displayNameLineEdit

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
                                        displayNameLineEdit.text)
                        }
                    }
                }

                // Identity
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

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Label {
                            id: idLabel
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: qsTr("Id")
                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        TextField {
                            id: currentRingID

                            property var backgroundColor: "transparent"
                            property var borderColor: "transparent"

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true
                            font.bold: true

                            readOnly: true
                            selectByMouse: true

                            text: currentRingIDText.elidedText

                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter

                            background: Rectangle {
                                anchors.fill: parent
                                radius: 0
                                border.color: currentRingID.borderColor
                                border.width: 0
                                color: currentRingID.backgroundColor
                            }

                            TextMetrics {
                                id: currentRingIDText

                                elide: Text.ElideRight
                                elideWidth: root.width - idLabel.width -JamiTheme.preferredMarginSize*4

                                text: ClientWrapper.SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        ElidedTextLabel {
                            id: lblRegisteredName
                            Layout.fillWidth: true
                            Layout.preferredWidth: preferredColumnWidth

                            eText: qsTr("Registered name")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }

                        UsernameLineEdit {
                            id: currentRegisteredID

                            Layout.alignment: Qt.AlignRight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.fillWidth: true

                            placeholderText: registeredIdNeedsSet ?
                                                 qsTr("Type here to register a username") : ""
                            text: {
                                if (!registeredIdNeedsSet)
                                    return ClientWrapper.SettingsAdapter.get_CurrentAccountInfo_RegisteredName()
                                else
                                    return ""
                            }
                            readOnly: !registeredIdNeedsSet
                            font.bold: !registeredIdNeedsSet

                            horizontalAlignment: registeredIdNeedsSet ?
                                                Text.AlignLeft :
                                                Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            padding: 8
                        }
                    }

                    MaterialButton {
                        id: btnRegisterName

                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        Layout.rightMargin: currentRegisteredID.width / 2 - width / 2
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 30

                        visible: registeredIdNeedsSet &&
                                 currentRegisteredID.nameRegistrationState ===
                                 UsernameLineEdit.NameRegistrationState.FREE

                        text: qsTr("Register")
                        toolTipText: qsTr("Register the username")
                        color: JamiTheme.buttonTintedGrey
                        hoveredColor: JamiTheme.buttonTintedGreyHovered
                        pressedColor: JamiTheme.buttonTintedGreyPressed

                        onClicked: nameRegistrationDialog.openNameRegistrationDialog(
                                       currentRegisteredID.text)
                    }
                }

                // Buttons Pwd, Export, Delete
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    MaterialButton {
                        id: passwdPushButton

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        visible: SettingsAdapter.getAccountConfig_Manageruri() === ""

                        color: JamiTheme.buttonTintedBlack
                        hoveredColor: JamiTheme.buttonTintedBlackHovered
                        pressedColor: JamiTheme.buttonTintedBlackPressed
                        outlined: true

                        toolTipText: ClientWrapper.accountAdaptor.hasPassword() ?
                                    qsTr("Change the current password") :
                                    qsTr("Currently no password, press this button to set a password")
                        text: ClientWrapper.accountAdaptor.hasPassword() ? qsTr("Change Password") :
                                                                           qsTr("Set Password")

                        source: "qrc:/images/icons/round-edit-24px.svg"

                        onClicked: {
                            passwordClicked()
                        }
                    }

                    MaterialButton {
                        id: btnExportAccount

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        visible: SettingsAdapter.getAccountConfig_Manageruri() === ""

                        color: JamiTheme.buttonTintedBlack
                        hoveredColor: JamiTheme.buttonTintedBlackHovered
                        pressedColor: JamiTheme.buttonTintedBlackPressed
                        outlined: true

                        toolTipText: qsTr("Press this button to export account to a .gz file")
                        text: qsTr("Export Account")

                        source: "qrc:/images/icons/round-save_alt-24px.svg"

                        onClicked: {
                            exportAccountSlot()
                        }
                    }

                    MaterialButton {
                        id: btnDeleteAccount

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

                // Linked devices
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    Label {
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("Linked Devices")

                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true
                    }

                    ColumnLayout {
                        id: linkedDevicesLayout
                        Layout.fillWidth: true

                        ListViewJami {
                            id: settingsListView

                            Layout.fillWidth: true
                            Layout.preferredHeight: 160

                            model: DeviceItemListModel{}

                            delegate: DeviceItemDelegate {
                                id: settingsListDelegate

                                implicitWidth: settingsListView.width
                                width: settingsListView.width
                                height: 70

                                deviceName: DeviceName
                                deviceId: DeviceID
                                isCurrent: IsCurrent

                                onClicked: {
                                    settingsListView.currentIndex = index
                                }

                                onBtnRemoveDeviceClicked:{
                                    removeDeviceSlot(index)
                                }
                            }
                        }


                        MaterialButton {
                            id: linkDevPushButton

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            visible: SettingsAdapter.getAccountConfig_Manageruri() === ""

                            color: JamiTheme.buttonTintedBlack
                            hoveredColor: JamiTheme.buttonTintedBlackHovered
                            pressedColor: JamiTheme.buttonTintedBlackPressed
                            outlined: true
                            toolTipText: qsTr("Press to link one more device with this account")

                            source: "qrc:/images/icons/round-add-24px.svg"

                            text: qsTr("Link Another Device")

                            onClicked: {
                                showLinkDevSlot()
                            }
                        }
                    }
                }

                // Banned contacts
                ColumnLayout {
                    id: bannedContactsLayoutWidget

                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.alignment: Qt.AlignHCenter

                    RowLayout {
                        Layout.fillWidth: true

                        ElidedTextLabel {

                            id: lblBannedContacts

                            Layout.fillWidth: true

                            eText: qsTr("Banned Contacts")
                            fontSize: JamiTheme.headerFontSize
                            maxWidth: root.width - bannedContactsBtn.width
                                      - JamiTheme.preferredMarginSize * 4
                        }

                        HoverableButtonTextItem {
                            id: bannedContactsBtn

                            Layout.alignment: Qt.AlignRight

                            Layout.preferredWidth: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            radius: height / 2

                            toolTipText: qsTr("press to open or hide display of banned contact")

                            source: bannedContactsListWidget.visible?
                                        "qrc:/images/icons/round-arrow_drop_up-24px.svg" :
                                        "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                            onClicked: {
                                toggleBannedContacts()
                            }
                        }
                    }


                    ColumnLayout {
                        id: bannedContactsListWidget

                        visible: false

                        ListViewJami {
                            id: bannedListWidget

                            Layout.fillWidth: true
                            Layout.preferredHeight: 160

                            model: BannedListModel{}

                            delegate: BannedItemDelegate{
                                id: bannedListDelegate

                                width: bannedListWidget.width
                                height: 74

                                contactName : ContactName
                                contactID: ContactID
                                contactPicture_base64: ContactPicture

                                onClicked: {
                                    bannedListWidget.currentIndex = index
                                }

                                onBtnReAddContactClicked: {
                                    unban(index)
                                }
                            }
                        }
                    }
                }

                // Advanced Settigs Button
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
                        maxWidth: root.width - advancedAccountSettingsPButton.width
                                  - JamiTheme.preferredMarginSize * 6
                    }

                    HoverableButtonTextItem {
                        id: advancedAccountSettingsPButton

                        Layout.preferredWidth: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.alignment: Qt.AlignHCenter

                        radius: height / 2

                        toolTipText: qsTr("Press to display or hide advance settings")

                        source: {
                            if (advanceSettingsView.visible) {
                                return "qrc:/images/icons/round-arrow_drop_up-24px.svg"
                            } else {
                                return "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                            }
                        }

                        onClicked: {
                            advanceSettingsView.visible = !advanceSettingsView.visible
                            if (advanceSettingsView.visible) {
                                advanceSettingsView.updateAccountInfoDisplayedAdvance()
                                accountScrollView.vScrollBar.position =
                                        rowAdvancedSettingsBtn.y / accountViewLayout.height
                            } else {
                                accountScrollView.vScrollBar.position = 0
                            }
                        }
                    }
                }

                // Advanced Settings
                AdvancedSettingsView {
                    id: advanceSettingsView
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
