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

import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property alias text_sipServernameEditAlias: sipServernameEdit.text
    property alias text_sipProxyEditAlias: sipProxyEdit.text
    property alias text_sipUsernameEditAlias: sipUsernameEdit.text
    property alias text_sipPasswordEditAlias: sipPasswordEdit.text
    property int preferredHeight: createSIPAccountPageColumnLayout.implicitHeight

    signal createAccount
    signal leavePage

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

    color: JamiTheme.backgroundColor

    onVisibleChanged: {
        if (visible)
            sipServernameEdit.focus = true
    }

    ColumnLayout {
        id: createSIPAccountPageColumnLayout

        spacing: layoutSpacing

        anchors.centerIn: parent

        RowLayout {
            spacing: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: backButtonMargins
            Layout.preferredWidth: createAccountButton.width

            Label {
                text: JamiStrings.configureExistingSIP
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                Layout.alignment: Qt.AlignRight

                text: JamiStrings.optional
                color: JamiTheme.whiteColor
                padding: 8

                background: Rectangle {
                    color: JamiTheme.wizardBlueButtons
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
            placeholderText: JamiStrings.server
            font.pointSize: 9
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipProxyEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: JamiStrings.proxy
            font.pointSize: 9
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipUsernameEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: JamiStrings.username
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
            placeholderText: JamiStrings.password
            font.pointSize: 9
            font.kerning: true
        }

        MaterialButton {
            id: createAccountButton

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: backButtonMargins
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.createSIPAccount
            color: JamiTheme.wizardBlueButtons
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                createAccount()
            }
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
