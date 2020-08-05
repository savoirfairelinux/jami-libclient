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

Rectangle {
    id: root

    property alias text_pinFromDeviceAlias: pinFromDevice.text
    property alias text_passwordFromDeviceAlias: passwordFromDevice.text
    property string errorText: ""

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        pinFromDevice.clear()
        passwordFromDevice.clear()
    }

    anchors.fill: parent

    color: JamiTheme.backgroundColor

    signal leavePage
    signal importAccount

    ColumnLayout {
        spacing: 12

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Layout.preferredWidth: parent.width
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        Text {
            anchors.left: connectBtn.left
            anchors.right: connectBtn.right

            text: qsTr("Enter your main Jami account password")
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialLineEdit {
            id: passwordFromDevice

            selectByMouse: true
            placeholderText: qsTr("Password")
            font.pointSize: 10
            font.kerning: true

            echoMode: TextInput.Password

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: connectBtn.width
        }

        Text {
            anchors.left: connectBtn.left
            anchors.right: connectBtn.right

            text: qsTr("Enter the PIN from another configured Jami account. Use the \"export Jami account\" feature to obtain a PIN")
            wrapMode: Text.Wrap
        }

        MaterialLineEdit {
            id: pinFromDevice

            selectByMouse: true
            placeholderText: qsTr("PIN")
            font.pointSize: 10
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: connectBtn.width
        }

        MaterialButton {
            id: connectBtn
            text: qsTr("CONNECT FROM ANOTHER DEVICE")
            color: pinFromDevice.text.length === 0?
                JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedGrey

            onClicked: {
                errorText = ""
                importAccount()
            }
        }

        Label {
            text: errorText

            anchors.left: connectBtn.left
            anchors.right: connectBtn.right
            Layout.alignment: Qt.AlignHCenter

            font.pointSize: JamiTheme.textFontSize
            color: "red"

            height: 32
        }
    }

    HoverableButton {
        id: cancelButton
        z: 2

        anchors.right: parent.right
        anchors.top: parent.top

        rightPadding: 90
        topPadding: 90

        Layout.preferredWidth: 96
        Layout.preferredHeight: 96

        backgroundColor: "transparent"
        onEnterColor: "transparent"
        onPressColor: "transparent"
        onReleaseColor: "transparent"
        onExitColor: "transparent"

        buttonImageHeight: 48
        buttonImageWidth: 48
        source: "qrc:/images/icons/ic_close_white_24dp.png"
        radius: 48
        baseColor: "#7c7c7c"
        toolTipText: qsTr("Return to welcome page")

        Shortcut {
            sequence: StandardKey.Cancel
            enabled: parent.visible
            onActivated: leavePage()
        }

        onClicked: {
            leavePage()
        }
    }
}
