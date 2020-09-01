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

    function initializeOnShowUp() {
        clearAllTextFields()
        boothImgBase64 = ""
        saveProfileBtn.spinnerTriggered = true
    }

    function clearAllTextFields() {
        aliasEdit.clear()
    }

    function readyToSaveDetails() {
        saveProfileBtn.spinnerTriggered = false
    }

    color: JamiTheme.backgroundColor

    signal leavePage
    signal saveProfile

    property var showBottom: false
    property alias boothImgBase64: setAvatarWidget.imgBase64
    property alias displayName: aliasEdit.text
    property bool isRdv: false

    ColumnLayout {
        spacing: layoutSpacing

        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            spacing: layoutSpacing

            Layout.preferredWidth: saveProfileBtn.width
            Layout.alignment: Qt.AlignCenter

            Label {
                text: qsTr("Profile is only shared with contacts")
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

        PhotoboothView {
            id: setAvatarWidget

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredWidth

            boothWidth: 200
        }

        MaterialLineEdit {
            id: aliasEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: fieldLayoutWidth
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: isRdv ? qsTr("Enter the rendez-vous's name") : qsTr("Enter your name")
            font.pointSize: 9
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: saveProfileBtn.width
        }

        SpinnerButton {
            id: saveProfileBtn

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            enabled: !spinnerTriggered
            normalText: qsTr("Save Profile")
            spinnerTriggeredtext: root.isRdv ? qsTr("Generating rendez-vous…") : qsTr("Generating account…")
            onClicked: saveProfile()
        }

        MaterialButton {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: qsTr("SKIP")
            enabled: saveProfileBtn.enabled
            color: enabled? JamiTheme.buttonTintedGrey : JamiTheme.buttonTintedGreyInactive
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed
            outlined: true

            onClicked: {
                leavePage()
            }
        }
    }

    AccountCreationStepIndicator {
        anchors.bottom: root.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: root.horizontalCenter

        spacing: layoutSpacing
        steps: 3
        currentStep: 3
    }
}
