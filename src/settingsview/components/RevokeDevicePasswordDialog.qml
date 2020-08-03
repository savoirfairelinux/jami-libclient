/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4

import "../../commoncomponents"

Dialog {
    id: revokeDevicePasswordDialog

    property string deviceId : ""

    signal revokeDeviceWithPassword(string idOfDevice, string password)

    function openRevokeDeviceDialog(deviceIdIn){
        deviceId = deviceIdIn
        passwordEdit.clear()
        revokeDevicePasswordDialog.open()
    }

    visible: false

    anchors.centerIn: parent.Center
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    title: qsTr("Enter this account's password to confirm the removal of this device")

    onClosed: {
        reject()
    }

    onAccepted:{
        revokeDeviceWithPassword(deviceId,passwordEdit.text)
    }

    contentItem: Rectangle{
        implicitWidth: 365
        implicitHeight: 120

        ColumnLayout{
            anchors.fill: parent
            spacing: 7

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Item{
                Layout.fillHeight: true

                Layout.maximumWidth: 20
                Layout.preferredWidth: 20
                Layout.minimumWidth: 20
            }

            InfoLineEdit{
                id: passwordEdit

                Layout.alignment: Qt.AlignHCenter

                Layout.minimumWidth: 294
                Layout.preferredWidth: 294

                Layout.preferredHeight: 30
                Layout.minimumHeight: 30

                echoMode: TextInput.Password

                placeholderText: qsTr("Password")
            }

            Item{
                Layout.fillHeight: true

                Layout.maximumWidth: 20
                Layout.preferredWidth: 20
                Layout.minimumWidth: 20
            }

            RowLayout{
                spacing: 7

                Layout.alignment: Qt.AlignHCenter

                Layout.fillWidth: true

                Item{
                    Layout.fillWidth: true

                    Layout.maximumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.minimumHeight: 20
                }

                HoverableRadiusButton{
                    id: btnOkay

                    Layout.maximumWidth: 130
                    Layout.preferredWidth: 130
                    Layout.minimumWidth: 130

                    Layout.maximumHeight: 30
                    Layout.preferredHeight: 30
                    Layout.minimumHeight: 30

                    radius: height /2

                    text: qsTr("Okay")
                    font.pointSize: 10
                    font.kerning: true

                    onClicked: {
                        accept()
                    }
                }

                Item{
                    Layout.fillWidth: true
                    Layout.minimumWidth: 40

                    Layout.maximumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.minimumHeight: 20
                }

                HoverableButtonTextItem {
                    id: btnCancel

                    Layout.maximumWidth: 130
                    Layout.preferredWidth: 130
                    Layout.minimumWidth: 130

                    Layout.maximumHeight: 30
                    Layout.preferredHeight: 30
                    Layout.minimumHeight: 30

                    backgroundColor: "red"
                    onEnterColor: Qt.rgba(150 / 256, 0, 0, 0.7)
                    onDisabledBackgroundColor: Qt.rgba(
                                                   255 / 256,
                                                   0, 0, 0.8)
                    onPressColor: backgroundColor
                    textColor: "white"

                    radius: height /2

                    text: qsTr("Cancel")
                    font.pointSize: 10
                    font.kerning: true

                    onClicked: {
                        reject()
                    }
                }

                Item{
                    Layout.fillWidth: true
                    Layout.minimumWidth: 40

                    Layout.maximumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.minimumHeight: 20
                }

            }

            Item{
                Layout.fillHeight: true

                Layout.maximumWidth: 20
                Layout.preferredWidth: 20
                Layout.minimumWidth: 20
            }
        }
    }
}
