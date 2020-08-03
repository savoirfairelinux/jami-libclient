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

import QtQuick 2.14
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14

import "../../constant"
import "../../commoncomponents"

ColumnLayout {
    property alias text_pinFromDeviceAlias: pinFromDevice.text
    property alias text_passwordFromDeviceAlias: passwordFromDevice.text

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        pinFromDevice.clear()
        passwordFromDevice.clear()
    }

    Layout.fillWidth: true
    Layout.fillHeight: true

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    ColumnLayout {
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true

        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 24
            spacing: 0

            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Label {
                id: importFromDeviceLabel
                Layout.minimumHeight: 24
                Layout.minimumWidth: 234
                text: qsTr("Import from device")
                font.pointSize: 13
                font.kerning: true
            }

            HoverableRadiusButton {
                id: pinInfoBtn

                buttonImageHeight: height
                buttonImageWidth: width

                Layout.alignment: Qt.AlignVCenter
                Layout.minimumWidth: 24
                Layout.maximumWidth: 24
                Layout.minimumHeight: 24
                Layout.maximumHeight: 24

                radius: height / 2
                icon.source: "/images/icons/info-24px.svg"
                backgroundColor: JamiTheme.releaseColor

                onClicked: {
                    pinInfoLabel.visible = !pinInfoLabel.visible
                }
            }
            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
        InfoLineEdit {
            id: pinFromDevice

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            placeholderText: qsTr("PIN")
        }

        InfoLineEdit {
            id: passwordFromDevice

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
        }

        Label {
            id: pinInfoLabel

            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: 256
            Layout.maximumWidth: 256

            text: qsTr("To obtain a PIN (valid for 10 minutes), you need to open the account settings on the other device and click on \"Link to another device\".")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            visible: false
        }
    }

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
