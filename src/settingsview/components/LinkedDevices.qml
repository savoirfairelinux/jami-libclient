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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

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

    Connections {
        id: accountConnections

        target: AccountAdapter
        enabled: root.visible

        function onAccountStatusChanged(id) {
            if (SettingsAdapter.getAccountConfig_Manageruri() === ""){
                linkDevPushButton.visible = SettingsAdapter.get_CurrentAccountInfo_Enabled()
            }
        }
    }

    function connectCurrentAccount(status) {
        accountConnections_DeviceModel.enabled = status
    }

    function updateAndShowDevicesSlot() {
        if (SettingsAdapter.getAccountConfig_Manageruri() === ""){
            linkDevPushButton.visible = SettingsAdapter.get_CurrentAccountInfo_Enabled()
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
        color: JamiTheme.textColor

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
    }

    ListViewJami {
        id: settingsListView

        Layout.fillWidth: true
        Layout.preferredHeight: 160

        model: DeviceItemListModel {
            lrcInstance: LRCInstance
        }

        delegate: DeviceItemDelegate {
            id: settingsListDelegate

            implicitWidth: settingsListView.width
            width: settingsListView.width
            height: 70

            deviceName: DeviceName
            deviceId: DeviceID
            isCurrent: IsCurrent

            onBtnRemoveDeviceClicked: removeDeviceSlot(index)
        }
    }

    MaterialButton {
        id: linkDevPushButton

        Layout.alignment: Qt.AlignCenter

        preferredWidth: JamiTheme.preferredFieldWidth

        color: JamiTheme.buttonTintedBlack
        hoveredColor: JamiTheme.buttonTintedBlackHovered
        pressedColor: JamiTheme.buttonTintedBlackPressed
        outlined: true
        toolTipText: JamiStrings.tipLinkNewDevice

        iconSource: JamiResources.round_add_24dp_svg

        text: JamiStrings.linkAnotherDevice

        onClicked: linkDeviceDialog.openLinkDeviceDialog()
    }
}
