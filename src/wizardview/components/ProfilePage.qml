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
        readyToSaveDetails = false
    }

    function clearAllTextFields() {
        aliasEdit.clear()
    }

    anchors.fill: parent

    color: JamiTheme.backgroundColor

    signal leavePage
    signal saveProfile

    property var readyToSaveDetails: false
    property var showBottom: false
    property alias boothImgBase64: setAvatarWidget.imgBase64
    property alias displayName: aliasEdit.text

    ColumnLayout {
        spacing: 12

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Layout.preferredWidth: parent.width
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter


        RowLayout {
            spacing: 12
            height: 48

            Layout.preferredWidth: saveProfileBtn.width

            Label {
                text: qsTr("Profile is only shared with contacts")

                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                text: qsTr("Optional")
                color: "white"
                Layout.alignment: Qt.AlignRight
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

            Layout.alignment: Qt.AlignHCenter

            Layout.maximumWidth: 256
            Layout.preferredWidth: 256
            Layout.minimumWidth: 256
            Layout.maximumHeight: 256
            Layout.preferredHeight: 256
            Layout.minimumHeight: 256
        }

        MaterialLineEdit {
            id: aliasEdit

            selectByMouse: true
            placeholderText: qsTr("Enter your name")
            font.pointSize: 10
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: saveProfileBtn.width
        }

        MaterialButton {
            id: saveProfileBtn
            enabled: readyToSaveDetails
            text: enabled? qsTr("Save Profile") : qsTr("Generating account…")
            color: enabled? JamiTheme.wizardBlueButtons : JamiTheme.buttonTintedGreyInactive
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                saveProfile()
            }
        }

        MaterialButton {
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

        RowLayout {
            id: bottomLayout
            height: 48
            spacing: 12
            visible: showBottom

            Layout.preferredWidth: saveProfileBtn.width
            Layout.topMargin: 12
            Layout.alignment: Qt.AlignHCenter

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                color: "grey"
                radius: height / 2
                height: 12
                width: 12
            }

            Rectangle {
                color: "grey"
                radius: height / 2
                height: 12
                width: 12
            }

            Rectangle {
                color: JamiTheme.wizardBlueButtons
                radius: height / 2
                height: 12
                width: 12
            }

            Item {
                Layout.fillWidth: true
            }
        }

    }

    HoverableButton {
        id: cancelButton
        z: 2
        visible: readyToSaveDetails

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
        toolTipText: qsTr("Close")

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
