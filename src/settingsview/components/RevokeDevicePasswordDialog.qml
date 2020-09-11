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
import "../../constant"
import "../../commoncomponents"

Dialog {
    id: root

    property string deviceId : ""

    signal revokeDeviceWithPassword(string idOfDevice, string password)

    function openRevokeDeviceDialog(deviceIdIn){
        deviceId = deviceIdIn
        passwordEdit.clear()
        root.open()
    }

    header : Rectangle {
        width: parent.width
        height: 64
        color: "transparent"
        Text {
            anchors.fill: parent
            anchors.leftMargin: JamiTheme.preferredMarginSize
            anchors.topMargin: JamiTheme.preferredMarginSize

            text: JamiStrings.confirmRemovalRequest
            wrapMode: Text.Wrap
            font.pointSize: JamiTheme.headerFontSize
        }
    }

    visible: false
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    onAccepted: {
        revokeDeviceWithPassword(deviceId,passwordEdit.text)
    }

    contentItem: Rectangle {
        implicitWidth: 350
        implicitHeight: contentLayout.implicitHeight + 64 + JamiTheme.preferredMarginSize

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            anchors.centerIn: parent

            MaterialLineEdit {
                id: passwordEdit

                Layout.preferredHeight: 48
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: 300

                echoMode: TextInput.Password
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                placeholderText: qsTr("Password")
            }

            RowLayout {
                Layout.topMargin: JamiTheme.preferredMarginSize / 2
                Layout.alignment: Qt.AlignRight

                Button {
                    id: btnChangePasswordConfirm

                    contentItem: Text {
                        text: qsTr("CONFIRM")
                        color: JamiTheme.buttonTintedBlue
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        timerToOperate.restart()
                    }
                }


                Button {
                    id: btnChangePasswordCancel
                    Layout.leftMargin: JamiTheme.preferredMarginSize / 2

                    contentItem: Text {
                        text: qsTr("CANCEL")
                        color: JamiTheme.buttonTintedBlue
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        root.reject()
                    }
                }
            }
        }
    }
}
