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
    id: accountViewRect

    enum RegName {
        BLANK,
        INVALIDFORM,
        TAKEN,
        FREE,
        SEARCHING
    }

    property int regNameUi: CurrentAccountSettingsScrollPage.BLANK
    property string registeredName: ""
    property bool registeredIdNeedsSet: false

    property int refreshVariable : 0
    property int preferredColumnWidth : accountViewRect.width / 2 - 50

    signal navigateToMainView
    signal navigateToNewWizardView
    signal backArrowClicked

    function refreshRelevantUI(){
        refreshVariable++
        refreshVariable--
    }

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

        bannedContactsLayoutWidget.visible = (bannedListModel.rowCount() > 0)

        if (advanceSettingsView.visible) {
            advanceSettingsView.updateAccountInfoDisplayedAdvance()
        }
        refreshRelevantUI()
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

    // slots
    function verifyRegisteredNameSlot() {
        if (ClientWrapper.SettingsAdapter.get_CurrentAccountInfo_RegisteredName() !== "") {
            regNameUi = CurrentAccountSettingsScrollPage.BLANK
        } else {
            registeredName = ClientWrapper.utilsAdaptor.stringSimplifier(
                        currentRegisteredID.text)
            if (registeredName !== "") {
                if (ClientWrapper.utilsAdaptor.validateRegNameForm(registeredName)) {
                    regNameUi = CurrentAccountSettingsScrollPage.SEARCHING
                    lookUpLabelTimer.restart()
                } else {
                    regNameUi = CurrentAccountSettingsScrollPage.INVALIDFORM
                }
            } else {
                regNameUi = CurrentAccountSettingsScrollPage.BLANK
            }
        }
    }

    Timer {
        id: lookUpLabelTimer

        interval: 300
        onTriggered: {
            beforeNameLookup()
        }
    }

    function beforeNameLookup() {
        ClientWrapper.nameDirectory.lookupName("", registeredName)
    }

    Connections {
        target: ClientWrapper.nameDirectory
        enabled: true

        function onRegisteredNameFound(status, address, name) {
            afterNameLookup(status, name)
        }
    }

    function afterNameLookup(status, regName) {
        if (registeredName === regName && regName.length > 2) {
            switch (status) {
            case NameDirectory.LookupStatus.NOT_FOUND:
                regNameUi = CurrentAccountSettingsScrollPage.FREE
                break
            default:
                regNameUi = CurrentAccountSettingsScrollPage.TAKEN
                break
            }
        } else {
            regNameUi = CurrentAccountSettingsScrollPage.BLANK
        }
    }

    function setAccEnableSlot(state) {
        ClientWrapper.accountModel.setAccountEnabled(ClientWrapper.utilsAdaptor.getCurrAccId(), state)
    }

    /*
     * JamiFileDialog for exporting account
     */
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
                    msgDialog.openWithParameters(title,info, iconMode, StandardButton.Ok)
                }
            }
        }

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }
    }

    function exportAccountSlot() {
        exportBtn_Dialog.open()
    }

    PasswordDialog {
        id: passwordDialog

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

            msgDialog.openWithParameters(title,info, iconMode, StandardButton.Ok)
        }
    }

    MessageBox {
        id: msgDialog
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

    DeleteAccountDialog{
        id: deleteAccountDialog

        anchors.centerIn: parent.Center
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

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
        }
    }

    function slotRegisterName() {
        refreshRelevantUI()
        nameRegistrationDialog.openNameRegistrationDialog(registeredName)
    }

    LinkDeviceDialog{
        id: linkDeviceDialog

        onAccepted: {
            updateAndShowDevicesSlot()
        }
    }

    function showLinkDevSlot() {
        linkDeviceDialog.openLinkDeviceDialog()
    }

    RevokeDevicePasswordDialog{
        id: revokeDevicePasswordDialog

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

        onYes: {
            accepted()
        }

        onNo:{
            rejected()
        }

        onDiscard: {
            rejected()
        }

        onAccepted: {
            revokeDeviceWithIDAndPassword(idOfDev,"")
        }

        onRejected: {}
    }

    function removeDeviceSlot(index){
        var idOfDevice = deviceItemListModel.data(deviceItemListModel.index(index,0), DeviceItemListModel.DeviceID)
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
        if(bannedListModel.rowCount() <= 0){
            bannedContactsLayoutWidget.visible = false
            return
        }

        bannedListModel.reset()
    }

    function updateAndShowDevicesSlot() {
        if(ClientWrapper.SettingsAdapter.getAccountConfig_Manageruri() === ""){
            linkDevPushButton.visible = true
        }

        deviceItemListModel.reset()
    }

    DeviceItemListModel {
        id: deviceItemListModel
    }

    BannedListModel{
        id: bannedListModel
    }

    Layout.fillHeight: true
    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView
    anchors.centerIn: parent

    ColumnLayout {
        anchors.fill: accountViewRect

        RowLayout {
            id: accountPageTitle
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
                maxWidth: !backToSettingsMenuButton.visible ? accountViewRect.width - 100 :
                                                              accountViewRect.width - backToSettingsMenuButton.width - 100

            }
        }

        ScrollView {
            id: accountScrollView

            property ScrollBar vScrollBar: ScrollBar.vertical

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            width: accountViewRect.width
            height: accountViewRect.height - accountPageTitle.height

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            clip: true

            /*
             * ScrollView Layout
             */
            ColumnLayout {
                id: accountViewLayout

                Layout.fillHeight: true
                Layout.preferredWidth: accountViewRect.width
                Layout.alignment: Qt.AlignHCenter

                spacing: 24

                ToggleSwitch {
                    id: accountEnableCheckBox

                    Layout.fillWidth: true
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    labelText: qsTr("Enable")
                    fontPointSize: JamiTheme.headerFontSize

                    onSwitchToggled: {
                        setAccEnableSlot(checked)
                    }
                }

                /*
                 * Profile
                 */
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

                    Label {
                        Layout.fillWidth: true

                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        text: qsTr("Profile")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    PhotoboothView {
                        id: currentAccountAvatar

                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                        boothWidth: Math.min(224, accountViewRect.width - 100)

                        Layout.maximumWidth: boothWidth+50
                        Layout.preferredWidth: boothWidth+50
                        Layout.minimumWidth: boothWidth+50
                        Layout.maximumHeight: boothWidth+50
                        Layout.preferredHeight: boothWidth+50
                        Layout.minimumHeight: boothWidth+50

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

                        Layout.maximumWidth: JamiTheme.preferredFieldWidth
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        Layout.alignment: Qt.AlignHCenter

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

                /*
                 * Identity
                 */
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
                        maxWidth: accountViewRect.width - 72
                        fontSize: JamiTheme.headerFontSize
                    }

                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Label {
                            id: idLabel
                            Layout.fillWidth: true
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                            text: { currentRingIDText.elidedText }

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
                                elideWidth: accountViewRect.width - idLabel.width -JamiTheme.preferredMarginSize*4

                                text: { refreshVariable
                                    return ClientWrapper.SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
                                }
                            }
                        }
                    }

                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            id: lblRegisteredName
                            Layout.fillWidth: true
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Registered name")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: preferredColumnWidth
                        }

                        MaterialLineEdit {
                            id: currentRegisteredID
                            Layout.minimumWidth: preferredColumnWidth
                            Layout.preferredWidth: preferredColumnWidth
                            Layout.maximumWidth: preferredColumnWidth
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            placeholderText: { refreshVariable
                                               var result = registeredIdNeedsSet ?
                                                   qsTr("Type here to register a username") : ""
                                               return result}

                            text: {
                                refreshVariable
                                if (!registeredIdNeedsSet){
                                    return ClientWrapper.SettingsAdapter.get_CurrentAccountInfo_RegisteredName()
                                } else {
                                    return ""
                                }
                            }
                            selectByMouse: true
                            readOnly: { refreshVariable
                                        return !registeredIdNeedsSet}

                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true
                            font.bold: { refreshVariable
                                return !registeredIdNeedsSet}

                            horizontalAlignment: registeredIdNeedsSet ?
                                                Text.AlignLeft :
                                                Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            padding: 8

                            borderColorMode: {
                                switch (regNameUi) {
                                case CurrentAccountSettingsScrollPage.INVALIDFORM:
                                case CurrentAccountSettingsScrollPage.TAKEN:
                                    return InfoLineEdit.ERROR
                                case CurrentAccountSettingsScrollPage.FREE:
                                    return InfoLineEdit.RIGHT
                                case CurrentAccountSettingsScrollPage.BLANK:
                                case CurrentAccountSettingsScrollPage.SEARCHING:
                                default:
                                    return InfoLineEdit.NORMAL
                                }
                            }

                            onImageClicked: {
                                slotRegisterName()
                            }

                            onTextEdited: {
                                verifyRegisteredNameSlot()
                            }

                            onEditingFinished: {
                                verifyRegisteredNameSlot()
                            }
                        }
                    }
                }

                /*
                 * Buttons Pwd, Export, Delete
                 */
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8

                    MaterialButton {
                        id: passwdPushButton

                        visible: SettingsAdapter.getAccountConfig_Manageruri() === ""

                        color: JamiTheme.buttonTintedBlack
                        hoveredColor: JamiTheme.buttonTintedBlackHovered
                        pressedColor: JamiTheme.buttonTintedBlackPressed
                        outlined: true

                        Layout.minimumHeight:  JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumWidth:  JamiTheme.preferredFieldWidth
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.maximumWidth: JamiTheme.preferredFieldWidth
                        Layout.alignment: Qt.AlignHCenter

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

                        visible: SettingsAdapter.getAccountConfig_Manageruri() === ""

                        color: JamiTheme.buttonTintedBlack
                        hoveredColor: JamiTheme.buttonTintedBlackHovered
                        pressedColor: JamiTheme.buttonTintedBlackPressed
                        outlined: true

                        Layout.minimumHeight:  JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumWidth:  JamiTheme.preferredFieldWidth
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.maximumWidth: JamiTheme.preferredFieldWidth
                        Layout.alignment: Qt.AlignHCenter

                        toolTipText: qsTr("Press this button to export account to a .gz file")
                        text: qsTr("Export Account")

                        source: "qrc:/images/icons/round-save_alt-24px.svg"

                        onClicked: {
                            exportAccountSlot()
                        }
                    }

                    MaterialButton {
                        id: btnDeleteAccount

                        color: JamiTheme.buttonTintedRed
                        hoveredColor: JamiTheme.buttonTintedRedHovered
                        pressedColor: JamiTheme.buttonTintedRedPressed

                        Layout.minimumHeight:  JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.minimumWidth:  JamiTheme.preferredFieldWidth
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.maximumWidth: JamiTheme.preferredFieldWidth
                        Layout.alignment: Qt.AlignHCenter

                        toolTipText: qsTr("Press this button to delete this account")
                        text: qsTr("Delete Account")

                        source: "qrc:/images/icons/delete_forever-24px.svg"

                        onClicked: {
                            delAccountSlot()
                        }
                    }
                }

                /*
                 Linked devices
                 */
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    Label {
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                            Layout.minimumHeight: 160
                            Layout.preferredHeight: 160
                            Layout.maximumHeight: 160

                            model: deviceItemListModel

                            delegate: DeviceItemDelegate{
                                id: settingsListDelegate

                                width: settingsListView.width
                                height: 72

                                deviceName : DeviceName
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

                            visible: SettingsAdapter.getAccountConfig_Manageruri() === ""

                            Layout.minimumHeight:  JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight
                            Layout.minimumWidth:  JamiTheme.preferredFieldWidth
                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.maximumWidth: JamiTheme.preferredFieldWidth
                            Layout.alignment: Qt.AlignCenter

                            color: JamiTheme.buttonTintedBlack
                            hoveredColor: JamiTheme.buttonTintedBlackHovered
                            pressedColor: JamiTheme.buttonTintedBlackPressed
                            outlined: true
                            toolTipText: qsTr("Press to link one more device with this account")

                            source: "qrc:/images/icons/round-add-24px.svg"

                            text: qsTr("Link Another Device")
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            onClicked: {
                                showLinkDevSlot()
                            }
                        }
                    }
                }

                /*
                 * Banned contacts
                 */
                ColumnLayout {
                    id: bannedContactsLayoutWidget

                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true

                        ElidedTextLabel {

                            id: lblBannedContacts

                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight

                            eText: qsTr("Banned Contacts")
                            fontSize: JamiTheme.headerFontSize
                            maxWidth: accountViewRect.width - bannedContactsBtn.width
                                      - JamiTheme.preferredMarginSize*4
                        }

                        HoverableButtonTextItem {
                            id: bannedContactsBtn

                            Layout.alignment: Qt.AlignRight

                            Layout.maximumWidth: JamiTheme.preferredFieldHeight
                            Layout.preferredWidth: JamiTheme.preferredFieldHeight
                            Layout.minimumWidth: JamiTheme.preferredFieldHeight

                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                        spacing: 8
                        visible: false

                        ListViewJami {
                            id: bannedListWidget

                            Layout.fillWidth: true

                            Layout.minimumHeight: 160
                            Layout.preferredHeight: 160
                            Layout.maximumHeight: 160

                            model: bannedListModel

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

                /*
                 * Advanced Settigs Button
                 */

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
                        maxWidth: accountViewRect.width - advancedAccountSettingsPButton.width
                                  - JamiTheme.preferredMarginSize*6
                    }

                    HoverableButtonTextItem {
                        id: advancedAccountSettingsPButton

                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Layout.minimumWidth: JamiTheme.preferredFieldHeight
                        Layout.preferredWidth: JamiTheme.preferredFieldHeight
                        Layout.maximumWidth: JamiTheme.preferredFieldHeight
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                /*
                 * Advanced Settings
                 */
                AdvancedSettingsView {
                    id: advanceSettingsView
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    visible: false
                }

                /*
                 * To keep max width
                 */
                Item {
                    Layout.preferredWidth: accountViewRect.width - 32
                    Layout.minimumWidth: accountViewRect.width - 32
                    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView - 32
                    Layout.fillHeight: true
                }
            }
        }
    }
}
