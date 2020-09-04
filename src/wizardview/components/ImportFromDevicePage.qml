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

    signal leavePage
    signal importAccount

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        connectBtn.spinnerTriggered = false
        pinFromDevice.clear()
        passwordFromDevice.clear()
    }

    function errorOccured(errorMessage) {
        errorText = errorMessage
        connectBtn.spinnerTriggered = false
    }

    color: JamiTheme.backgroundColor

    onVisibleChanged: {
        if (visible)
            pinFromDevice.focus = true
    }

    ColumnLayout {
        spacing: layoutSpacing

        // Prevent possible anchor loop detected on centerIn.
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Text {
            Layout.alignment: Qt.AlignCenter

            text: qsTr("Enter your main Jami account password")
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialLineEdit {
            id: passwordFromDevice

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: qsTr("Password")
            font.pointSize: 9
            font.kerning: true

            echoMode: TextInput.Password
            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        Text {
            property int preferredHeight: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: connectBtn.width
            Layout.preferredHeight: preferredHeight

            text: qsTr("Enter the PIN from another configured Jami account. " +
                       "Use the \"Link Another Device\" feature to obtain a PIN")
            wrapMode: Text.Wrap

            onTextChanged: {
                var boundingRect = JamiQmlUtils.getTextBoundingRect(font, text)
                preferredHeight += (boundingRect.width / connectBtn.preferredWidth)
                        * boundingRect.height
            }
        }

        MaterialLineEdit {
            id: pinFromDevice

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: qsTr("PIN")
            font.pointSize: 9
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        SpinnerButton {
            id: connectBtn

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            spinnerTriggeredtext: qsTr("Generating account…")
            normalText: qsTr("CONNECT FROM ANOTHER DEVICE")

            enabled: pinFromDevice.text.length !== 0 && !spinnerTriggered

            onClicked: {
                spinnerTriggered = true
                importAccount()
            }
        }

        Label {
            Layout.alignment: Qt.AlignCenter

            visible: errorText.length !== 0

            text: errorText

            font.pointSize: JamiTheme.textFontSize
            color: "red"
        }
    }

    HoverableButton {
        id: backButton

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        width: 35
        height: 35

        radius: 30

        backgroundColor: root.color
        onExitColor: root.color

        source: "qrc:/images/icons/ic_arrow_back_24px.svg"
        toolTipText: qsTr("Back to welcome page")

        onClicked: leavePage()
    }
}
