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

    property var boothImgBase64: null

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

    color: JamiTheme.backgroundColor

    ColumnLayout {
        spacing: layoutSpacing

        anchors.centerIn: parent

        RowLayout {
            spacing: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: createAccountButton.width

            Label {
                text: qsTr("Configure an existing SIP account")
                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                Layout.alignment: Qt.AlignRight

                text: qsTr("Optional")
                color: "white"
                padding: 8

                background: Rectangle {
                    color: "#28b1ed"
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        MaterialLineEdit {
            id: sipServernameEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: qsTr("Server")
            font.pointSize: 9
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipProxyEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: qsTr("Proxy")
            font.pointSize: 9
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipUsernameEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: qsTr("Username")
            font.pointSize: 9
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipPasswordEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
            font.pointSize: 9
            font.kerning: true
        }

        MaterialButton {
            id: createAccountButton

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: qsTr("CREATE SIP ACCOUNT")
            color: JamiTheme.wizardBlueButtons
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                createAccount()
            }
        }

        MaterialButton {
            id: backButton

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: createAccountButton.width / 2
            Layout.preferredHeight: preferredHeight

            text: qsTr("BACK")
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed
            outlined: true

            onClicked: leavePage()
        }
    }
}
