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

    property alias text_usernameManagerEditAlias: usernameManagerEdit.text
    property alias text_passwordManagerEditAlias: passwordManagerEdit.text
    property alias text_accountManagerEditAlias: accountManagerEdit.text
    property string errorText: ""

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        usernameManagerEdit.clear()
        passwordManagerEdit.clear()
        accountManagerEdit.clear()
        errorText = ""
    }

    anchors.fill: parent

    color: JamiTheme.backgroundColor

    signal leavePage
    signal createAccount

    ColumnLayout {
        spacing: 12

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Layout.preferredWidth: parent.width
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        RowLayout {
            spacing: 12
            height: 48

            Layout.fillWidth: true
            anchors.left: connectBtn.left
            anchors.right: connectBtn.right

            Label {
                text: qsTr("Enter URL of management server")
            }

            Label {
                text: qsTr("Required")
                color: "#ff1f62"
                padding: 8
                anchors.right: parent.right

                background: Rectangle {
                    color: "#fee4e9"
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        MaterialLineEdit {
            id: accountManagerEdit

            selectByMouse: true
            placeholderText: qsTr("Jami management server URL")
            font.pointSize: 10
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: connectBtn.width
        }

        Text {
            anchors.left: connectBtn.left
            anchors.right: connectBtn.right

            text: qsTr("Enter your organization credentials")
            wrapMode: Text.Wrap
        }

        MaterialLineEdit {
            id: usernameManagerEdit

            selectByMouse: true
            placeholderText: qsTr("Username")
            font.pointSize: 10
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: connectBtn.width
        }

        MaterialLineEdit {
            id: passwordManagerEdit

            selectByMouse: true
            placeholderText: qsTr("Password")
            font.pointSize: 10
            font.kerning: true

            echoMode: TextInput.Password

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: connectBtn.width
        }

        MaterialButton {
            id: connectBtn
            text: qsTr("CONNECT")
            enabled: accountManagerEdit.text.length !== 0
                && usernameManagerEdit.text.length !== 0
                && passwordManagerEdit.text.length !== 0
            color: enabled? JamiTheme.wizardBlueButtons : JamiTheme.buttonTintedGreyInactive
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                errorText = ""
                createAccount()
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

        Action {
            enabled: parent.visible
            shortcut: StandardKey.Cancel
            onTriggered: leavePage()
        }

        onClicked: {
            leavePage()
        }
    }
}
