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

    property alias text_sipServernameEditAlias: sipServernameEdit.text
    property alias text_sipProxyEditAlias: sipProxyEdit.text
    property alias text_sipUsernameEditAlias: sipUsernameEdit.text
    property alias text_sipPasswordEditAlias: sipPasswordEdit.text

    property /*alias*/ var boothImgBase64: null//setSIPAvatarWidget.imgBase64

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        sipUsernameEdit.clear()
        sipPasswordEdit.clear()
        sipServernameEdit.clear()
        sipProxyEdit.clear()
        sipUsernameEdit.clear()
    }

    signal createAccount
    signal leavePage

    anchors.fill: parent

    color: JamiTheme.backgroundColor

    ColumnLayout {
        spacing: 12

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Layout.preferredWidth: createAccountButton.width
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        RowLayout {
            spacing: 12
            height: 48

            anchors.left: createAccountButton.left
            anchors.right: createAccountButton.right

            Label {
                text: qsTr("Configure an existing SIP account")
            }

            Label {
                text: qsTr("Required")
                color: "#ff1f62"
                padding: 8

                background: Rectangle {
                    color: "#fee4e9"
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        MaterialLineEdit {
            id: sipServernameEdit

            fieldLayoutWidth: createAccountButton.width

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            placeholderText: qsTr("Server")
            font.pointSize: 10
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipProxyEdit

            fieldLayoutWidth: createAccountButton.width

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            placeholderText: qsTr("Proxy")
            font.pointSize: 10
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipUsernameEdit

            fieldLayoutWidth: createAccountButton.width

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            placeholderText: qsTr("Username")
            font.pointSize: 10
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipPasswordEdit

            fieldLayoutWidth: createAccountButton.width

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
            font.pointSize: 10
            font.kerning: true
        }

        MaterialButton {
            id: createAccountButton
            text: qsTr("CREATE SIP ACCOUNT")
            color: JamiTheme.wizardBlueButtons

            onClicked: {
                createAccount()
            }
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
