/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import Qt.labs.platform 1.1
import "../../commoncomponents"

ColumnLayout {
    id:root

    Connections {
        id: accountConnections_DeviceModel
        target: AccountAdapter.deviceModel
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

    function connectCurrentAccount(status) {
        accountConnections_DeviceModel.enabled = status
    }

    function updateAndShowDevicesSlot() {
        if(SettingsAdapter.getAccountConfig_Manageruri() === ""){
            linkDevPushButton.visible = true
        }
        settingsListView.model.reset()
    }

    function revokeDeviceWithIDAndPassword(idDevice, password){
        AccountAdapter.deviceModel.revokeDevice(idDevice, password)
        updateAndShowDevicesSlot()
    }

    function removeDeviceSlot(index){
        var idOfDevice = settingsListView.model.data(settingsListView.model.index(index,0), DeviceItemListModel.DeviceID)
        if(AccountAdapter.hasPassword()){
            revokeDevicePasswordDialog.openRevokeDeviceDialog(idOfDevice)
        } else {
            revokeDeviceMessageBox.idOfDev = idOfDevice
            revokeDeviceMessageBox.open()
        }
    }

    LinkDeviceDialog {
        id: linkDeviceDialog

        onAccepted: updateAndShowDevicesSlot()
    }

    RevokeDevicePasswordDialog{
        id: revokeDevicePasswordDialog

        onRevokeDeviceWithPassword: revokeDeviceWithIDAndPassword(idOfDevice, password)
    }

    SimpleMessageDialog {
        id: revokeDeviceMessageBox

        property string idOfDev: ""

        title: qsTr("Remove Device")
        infoText: qsTr("Are you sure you wish to remove this device?")

        buttonTitles: [qsTr("Ok"), qsTr("Cancel")]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue,
                       SimpleMessageDialog.ButtonStyle.TintedBlack]
        buttonCallBacks: [function() {revokeDeviceWithIDAndPassword(idOfDev, "")}]
    }

    Label {
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        text: qsTr("Linked Devices")

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
    }

    ListViewJami {
        id: settingsListView

        Layout.fillWidth: true
        Layout.preferredHeight: 160

        model: DeviceItemListModel {}

        delegate: DeviceItemDelegate {
            id: settingsListDelegate

            implicitWidth: settingsListView.width
            width: settingsListView.width
            height: 70

            deviceName: DeviceName
            deviceId: DeviceID
            isCurrent: IsCurrent

            onClicked: settingsListView.currentIndex = index

            onBtnRemoveDeviceClicked: removeDeviceSlot(index)
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
        toolTipText: JamiStrings.tipLinkNewDevice

        source: "qrc:/images/icons/round-add-24px.svg"

        text: JamiStrings.linkAnotherDevice

        onClicked: linkDeviceDialog.openLinkDeviceDialog()
    }
}
