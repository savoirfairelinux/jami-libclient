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
    signal neverShowAgainBoxClicked(bool isChecked)
    signal skip_Btn_Clicked
    signal export_Btn_FileDialogAccepted(bool accepted, string folderDir)

    /*
     * JamiFileDialog for exporting account
     */
    JamiFileDialog {
        id: exportBtn_Dialog

        mode: JamiFileDialog.SaveFile

        title: qsTr("Export Account Here")
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [qsTr("Jami archive files") + " (*.gz)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            export_Btn_FileDialogAccepted(true, file)
        }

        onRejected: {
            export_Btn_FileDialogAccepted(false, folder)
        }

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }
    }

    Layout.fillWidth: true
    Layout.fillHeight: true

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    /*
     * Main layout for BackupKeyPage which consists of the buttons and "never show again" check box
     */
    ColumnLayout {
        Layout.alignment: Qt.AlignCenter
        Layout.maximumWidth: 366

        spacing: 12

        Label {
            id: backupKeysLabel
            Layout.alignment: Qt.AlignHCenter

            Layout.maximumWidth: 366
            Layout.maximumHeight: 21
            Layout.preferredWidth: 366
            Layout.preferredHeight: 21

            text: qsTr("Backup your account")
            font.pointSize: 13
            font.kerning: true
        }
        Label {
            id: backupInfoLabel1
            Layout.maximumWidth: 366
            Layout.maximumHeight: 57
            Layout.preferredWidth: 366
            Layout.preferredHeight: 57

            text: qsTr("This account only exists on this device. If you lost your device or uninstall the application,your account will be deleted. You can backup your account now or later.")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            verticalAlignment: Text.AlignVCenter
        }
        CheckBox {
            id: neverShowAgainBox
            checked: false

            Layout.maximumWidth: 366
            Layout.maximumHeight: 19
            Layout.preferredWidth: 366
            Layout.preferredHeight: 19

            indicator.implicitWidth: 10
            indicator.implicitHeight:10

            text: qsTr("Never show me this again")
            font.pointSize: 8

            onClicked: {
                neverShowAgainBoxClicked(checked)
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 20

            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillHeight: true
                Layout.maximumWidth: 40
                Layout.minimumWidth: 10
            }

            HoverableGradientButton {
                id: exportBtn

                Layout.alignment: Qt.AlignVCenter
                Layout.minimumWidth: 85
                Layout.preferredWidth: 85
                Layout.maximumWidth: 85

                Layout.minimumHeight: 30
                Layout.preferredHeight: 30
                Layout.maximumHeight: 30

                text: qsTr("Export")
                font.kerning: true
                fontPointSize: 10
                radius: height / 2
                backgroundColor: JamiTheme.releaseColor

                onClicked: {
                    exportBtn_Dialog.open()
                }
            }

            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            HoverableGradientButton {
                id: skipBtn

                Layout.alignment: Qt.AlignVCenter
                Layout.minimumWidth: 85
                Layout.preferredWidth: 85
                Layout.maximumWidth: 85

                Layout.minimumHeight: 30
                Layout.preferredHeight: 30
                Layout.maximumHeight: 30

                text: qsTr("Skip")
                fontPointSize: 10
                font.kerning: true
                radius: height / 2
                backgroundColor: JamiTheme.releaseColor

                onClicked: {
                    skip_Btn_Clicked()
                }
            }

            Item {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillHeight: true
                Layout.maximumWidth: 40
                Layout.minimumWidth: 10
            }
        }
    }

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
