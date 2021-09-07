/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import net.jami.Constants 1.1

import "../../commoncomponents"

BaseDialog {
    id: root

    property string deviceId : ""

    signal revokeDeviceWithPassword(string idOfDevice, string password)

    function openRevokeDeviceDialog(deviceIdIn) {
        deviceId = deviceIdIn
        passwordEdit.clear()
        open()
    }

    title: qsTr("Remove device")

    contentItem: Rectangle {
        id: revokeDeviceContentRect

        color: JamiTheme.secondaryBackgroundColor
        implicitWidth: JamiTheme.preferredDialogWidth
        implicitHeight: JamiTheme.preferredDialogHeight

        ColumnLayout {
            anchors.centerIn: parent
            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize
            spacing: 16

            Label {
                id: labelDeletion

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: revokeDeviceContentRect.width - JamiTheme.preferredMarginSize * 2

                text: qsTr("Enter this account's password to confirm the removal of this device")
                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                wrapMode: Text.Wrap

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: passwordEdit

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: JamiTheme.preferredFieldWidth
                Layout.preferredHeight: visible ? 48 : 0

                echoMode: TextInput.Password
                placeholderText: JamiStrings.enterCurrentPassword

                onTextChanged: {
                    btnRemove.enabled = text.length > 0
                }
            }

            RowLayout {
                spacing: 16
                Layout.alignment: Qt.AlignHCenter

                Layout.fillWidth: true

                MaterialButton {
                    id: btnRemove

                    Layout.alignment: Qt.AlignHCenter

                    preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                    preferredHeight: JamiTheme.preferredFieldHeight

                    color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    outlined: true
                    enabled: false

                    text: qsTr("Remove")

                    onClicked: {
                        revokeDeviceWithPassword(deviceId, passwordEdit.text)
                        close()
                    }
                }

                MaterialButton {
                    id: btnCancel

                    Layout.alignment: Qt.AlignHCenter

                    preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                    preferredHeight: JamiTheme.preferredFieldHeight

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    outlined: true
                    enabled: true

                    text: qsTr("Cancel")

                    onClicked: {
                        close()
                    }
                }
	    }
        }
    }
}
