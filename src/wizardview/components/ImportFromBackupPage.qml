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
import net.jami.Adapters 1.0

import "../../constant"
import "../../commoncomponents"

Rectangle {
    id: root

    property alias text_passwordFromBackupEditAlias: passwordFromBackupEdit.text
    property string fileImportBtnText: JamiStrings.archive
    property int preferredHeight: importFromBackupPageColumnLayout.implicitHeight

    property string filePath: ""
    property string errorText: ""

    signal leavePage
    signal importAccount

    function clearAllTextFields() {
        connectBtn.spinnerTriggered = false
        passwordFromBackupEdit.clear()
        errorText = ""
        fileImportBtnText = JamiStrings.archive
    }

    function errorOccured(errorMessage) {
        errorText = errorMessage
        connectBtn.spinnerTriggered = false
    }

    color: JamiTheme.backgroundColor

    JamiFileDialog {
        id: importFromFile_Dialog

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.openFile
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr("All files") + " (*)"]

        onAccepted: {
            filePath = file
            if (file.length !== "") {
                fileImportBtnText = UtilsAdapter.toFileInfoName(file)
            } else {
                fileImportBtnText = JamiStrings.archive
            }
        }
    }

    ColumnLayout {
        id: importFromBackupPageColumnLayout

        spacing: layoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Text {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: backButtonMargins

            text: qsTr("Import from backup")
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialButton {
            id: fileImportBtn

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: fileImportBtnText
            toolTipText: JamiStrings.importAccountArchive
            source: "qrc:/images/icons/round-folder-24px.svg"
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: {
                errorText = ""
                importFromFile_Dialog.open()
            }
        }

        Text {
            // For multiline text, recursive rearrange warning will show up when
            // directly assigning contentHeight to Layout.preferredHeight
            property int preferredHeight: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: fileImportBtn.width
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.importAccountExplanation
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

            onTextChanged: errorText = ""
        }

        SpinnerButton {
            id: connectBtn

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: errorLabel.visible ? 0 : backButtonMargins
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            spinnerTriggeredtext: qsTr("Generating account…")
            normalText: JamiStrings.connectFromBackup

            enabled: {
                if (spinnerTriggered)
                    return false
                if (!(filePath.length === 0) && errorText.length === 0)
                    return true
                return false
            }

            onClicked: {
                spinnerTriggered = true
                importAccount()
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
