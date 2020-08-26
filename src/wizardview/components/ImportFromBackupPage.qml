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
import Qt.labs.platform 1.1
import net.jami.Models 1.0

import "../../constant"
import "../../commoncomponents"

Rectangle {
    id: root

    property alias text_passwordFromBackupEditAlias: passwordFromBackupEdit.text
    property string fileImportBtnText: qsTr("Archive(none)")

    property string filePath: ""
    property string errorText: ""

    function clearAllTextFields() {
        passwordFromBackupEdit.clear()
        errorText = ""
        fileImportBtnText = qsTr("Archive(none)")
    }

    JamiFileDialog {
        id: importFromFile_Dialog

        mode: JamiFileDialog.OpenFile
        title: qsTr("Open File")
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr("All files") + " (*)"]

        onAccepted: {
            filePath = file
            if (file.length != 0) {
                fileImportBtnText = ClientWrapper.utilsAdaptor.toFileInfoName(file)
            } else {
                fileImportBtnText = qsTr("Archive(none)")
            }
        }
    }

    anchors.fill: parent

    color: JamiTheme.backgroundColor

    signal leavePage
    signal importAccount

    ColumnLayout {
        spacing: 12

        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter
        Layout.preferredWidth: parent.width
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        Text {
            anchors.left: connectBtn.left
            anchors.right: connectBtn.right

            text: qsTr("Import from backup")
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialButton {
            id: fileImportBtn

            text: fileImportBtnText
            toolTipText: qsTr("Import your account's archive")
            source: "qrc:/images/icons/round-folder-24px.svg"
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: {
                importFromFile_Dialog.open()
            }
        }

        Text {
            anchors.left: connectBtn.left
            anchors.right: connectBtn.right

            text: qsTr("You can obtain an archive by clicking on \"Export account\" in the account settings. This will create a .gz file on your device.")
            wrapMode: Text.Wrap
        }

        MaterialLineEdit {
            id: passwordFromBackupEdit

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
            text: qsTr("CONNECT FROM BACKUP")
            color: filePath.length === 0?
                JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

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
