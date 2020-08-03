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

import "../../constant"
import "../../commoncomponents"

ColumnLayout {
    property alias text_passwordFromBackupEditAlias: passwordFromBackupEdit.text
    property string fileImportBtnText: qsTr("Archive(none)")

    signal importFromFile_Dialog_Accepted(string fileDir)

    function clearAllTextFields() {
        passwordFromBackupEdit.clear()
        fileImportBtnText = qsTr("Archive(none)")
    }

    JamiFileDialog {
        id: importFromFile_Dialog

        mode: JamiFileDialog.OpenFile
        title: qsTr("Open File")
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            importFromFile_Dialog_Accepted(file)
        }
    }

    Layout.fillWidth: true
    Layout.fillHeight: true

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    ColumnLayout {
        Layout.alignment: Qt.AlignCenter
        Layout.maximumWidth: 366

        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 24
            spacing: 0

            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Label {
                id: importFromBackupLabel
                Layout.minimumHeight: 24
                Layout.minimumWidth: 234
                text: qsTr("Import from backup")
                font.pointSize: 13
                font.kerning: true
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
            }

            HoverableRadiusButton {
                id: backupInfoBtn

                buttonImageHeight: height
                buttonImageWidth: width

                Layout.alignment: Qt.AlignVCenter
                Layout.minimumWidth: 24
                Layout.preferredWidth: 24
                Layout.maximumWidth: 24

                Layout.minimumHeight: 24
                Layout.preferredHeight: 24
                Layout.maximumHeight: 24

                radius: height / 2
                icon.source: "/images/icons/info-24px.svg"
                icon.height: 24
                icon.width: 24

                backgroundColor: JamiTheme.releaseColor
                onClicked: {
                    backupInfoLabel.visible = !backupInfoLabel.visible
                }
            }
            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        HoverableGradientButton {
            id: fileImportBtn

            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 256
            Layout.preferredWidth: 256

            Layout.maximumHeight: 30
            Layout.preferredHeight: 30
            Layout.minimumHeight: 30

            text: fileImportBtnText
            font.pointSize: 10
            font.kerning: true

            radius: height / 2
            backgroundColor: JamiTheme.releaseColor

            onClicked: {
                importFromFile_Dialog.open()
            }
        }

        InfoLineEdit {
            id: passwordFromBackupEdit

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
        }

        Label {
            id: backupInfoLabel

            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 366
            Layout.preferredWidth: 366

            text: qsTr("You can obtain an archive by clicking on \"Export account\" in the account settings. This will create a .gz file on your device.")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            visible: false
        }
    }

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
