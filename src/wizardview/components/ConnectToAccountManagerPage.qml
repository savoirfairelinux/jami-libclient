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
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import "../../constant"
import "../../commoncomponents"

Rectangle {
    id: root

    property alias text_usernameManagerEditAlias: usernameManagerEdit.text
    property alias text_passwordManagerEditAlias: passwordManagerEdit.text
    property alias text_accountManagerEditAlias: accountManagerEdit.text
    property string errorText: ""
    property int preferredHeight: connectToAccountManagerPageColumnLayout.implicitHeight

    signal leavePage
    signal createAccount

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        connectBtn.spinnerTriggered = false
        usernameManagerEdit.clear()
        passwordManagerEdit.clear()
        accountManagerEdit.clear()
        errorText = ""
    }

    function errorOccured(errorMessage) {
        connectBtn.spinnerTriggered = false
        errorText = errorMessage
    }

    color: JamiTheme.backgroundColor

    onVisibleChanged: {
        if (visible)
            accountManagerEdit.focus = true
    }

    ColumnLayout {
        id: connectToAccountManagerPageColumnLayout

        spacing: layoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            spacing: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: backButtonMargins
            Layout.preferredWidth: implicitWidth

            Label {
                text: JamiStrings.enterJAMSURL
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                Layout.alignment: Qt.AlignRight

                text: JamiStrings.required
                color: JamiTheme.requiredFieldColor
                padding: 8

                background: Rectangle {
                    color: JamiTheme.requiredFieldBackgroundColor
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        MaterialLineEdit {
            id: accountManagerEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: JamiStrings.jamiManagementServerURL
            font.pointSize: 9
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: connectBtn.width

            text: JamiStrings.jamsCredentials
            color: JamiTheme.textColor
            wrapMode: Text.Wrap

            onTextChanged: Layout.preferredHeight =
                           JamiQmlUtils.getTextBoundingRect(font, text).height
        }

        MaterialLineEdit {
            id: usernameManagerEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: JamiStrings.username
            font.pointSize: 9
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        MaterialLineEdit {
            id: passwordManagerEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: JamiStrings.password
            font.pointSize: 9
            font.kerning: true

            echoMode: TextInput.Password
            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        SpinnerButton {
            id: connectBtn

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: errorLabel.visible ? 0 : backButtonMargins
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            spinnerTriggeredtext: JamiStrings.generatingAccount
            normalText: JamiStrings.connect

            enabled: accountManagerEdit.text.length !== 0
                     && usernameManagerEdit.text.length !== 0
                     && passwordManagerEdit.text.length !== 0
                     && !spinnerTriggered

            onClicked: {
                spinnerTriggered = true
                createAccount()
            }
        }

        Label {
            id: errorLabel

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: backButtonMargins

            visible: errorText.length !== 0
            text: errorText

            font.pointSize: JamiTheme.textFontSize
            color: "red"
        }
    }

    PushButton {
        id: backButton

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        width: 35
        height: 35

        normalColor: root.color
        imageColor: JamiTheme.primaryForegroundColor

        source: "qrc:/images/icons/ic_arrow_back_24px.svg"
        toolTipText: JamiStrings.backToWelcome

        onClicked: leavePage()
    }
}
