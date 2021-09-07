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
import net.jami.Enums 1.1

import "../../commoncomponents"

ColumnLayout {
    id:root

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

    function removeDeviceSlot(index){
        var idOfDevice = settingsListView.model.data(settingsListView.model.index(index,0),
                                                     DeviceItemListModel.DeviceID)
        if(AccountAdapter.hasPassword()){
            revokeDevicePasswordDialog.openRevokeDeviceDialog(idOfDevice)
        } else {
            revokeDeviceMessageBox.idOfDev = idOfDevice
            revokeDeviceMessageBox.open()
        }
    }

    LinkDeviceDialog {
        id: linkDeviceDialog
    }

    RevokeDevicePasswordDialog{
        id: revokeDevicePasswordDialog

        onRevokeDeviceWithPassword: deviceItemListModel.sourceModel.revokeDevice(idOfDevice, password)
    }

    SimpleMessageDialog {
        id: revokeDeviceMessageBox

        property string idOfDev: ""

        title: JamiStrings.removeDevice
        infoText: JamiStrings.sureToRemoveDevice

        buttonTitles: [JamiStrings.optionOk, JamiStrings.optionCancel]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue,
                       SimpleMessageDialog.ButtonStyle.TintedBlack]
        buttonCallBacks: [function() {deviceItemListModel.sourceModel.revokeDevice(idOfDev, "")}]
    }

    Label {
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        text: JamiStrings.linkedDevices
        color: JamiTheme.textColor

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
    }

    ListViewJami {
        id: settingsListView

        Layout.fillWidth: true
        Layout.preferredHeight: 160

        model: DeviceItemProxyModel {
            id: deviceItemListModel

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
