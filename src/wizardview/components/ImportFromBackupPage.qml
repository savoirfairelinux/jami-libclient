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

    color: JamiTheme.backgroundColor

    signal leavePage
    signal importAccount

    ColumnLayout {
        spacing: layoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Text {
            Layout.alignment: Qt.AlignCenter

            text: qsTr("Import from backup")
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialButton {
            id: fileImportBtn

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: fileImportBtnText
            toolTipText: qsTr("Import your account's archive")
            source: "qrc:/images/icons/round-folder-24px.svg"
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: importFromFile_Dialog.open()
        }

        Text {
            // For multiline text, recursive rearrange warning will show up when
            // directly assigning contentHeight to Layout.preferredHeight
            property int preferredHeight: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: fileImportBtn.width
            Layout.preferredHeight: preferredHeight

            text: qsTr("You can obtain an archive by clicking on \"Export account\" " +
                       "in the account settings. " +
                       "This will create a .gz file on your device.")
            wrapMode: Text.Wrap

            onTextChanged: {
                var boundingRect = JamiQmlUtils.getTextBoundingRect(font, text)
                preferredHeight += (boundingRect.width / fileImportBtn.preferredWidth)
                        * boundingRect.height
            }
        }

        MaterialLineEdit {
            id: passwordFromBackupEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: qsTr("Password")
            font.pointSize: 9
            font.kerning: true

            echoMode: TextInput.Password

            borderColorMode: MaterialLineEdit.NORMAL
        }

        MaterialButton {
            id: connectBtn

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: qsTr("CONNECT FROM BACKUP")
            color: filePath.length === 0 ?
                JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedGrey
            enabled: !(filePath.length === 0)
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: {
                errorText = ""
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

        MaterialButton {
            id: backButton

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: connectBtn.width / 2
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
