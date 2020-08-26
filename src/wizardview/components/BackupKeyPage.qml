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
import "../../settingsview/components"

Rectangle {
    id: root

    signal neverShowAgainBoxClicked(bool isChecked)
    signal leavePage
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

    anchors.fill: parent

    color: JamiTheme.backgroundColor

    ColumnLayout {
        spacing: 12

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Layout.preferredWidth: backupBtn.width
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        RowLayout {
            spacing: 12
            height: 48

            anchors.left: backupBtn.left
            anchors.right: backupBtn.right

            Label {
                text: qsTr("Backup your account!")

                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                text: qsTr("Recommended")
                color: "white"
                padding: 8
                anchors.right: parent.right

                background: Rectangle {
                    color: "#aed581"
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        Label {
            text: qsTr("This account only exists on this device. If you lost your device or uninstall the application, your account will be deleted. You can backup your account now or later.")
            wrapMode: Text.Wrap
            anchors.left: backupBtn.left
            anchors.right: backupBtn.right

            font.pointSize: JamiTheme.textFontSize
        }

        RowLayout {
            spacing: 12
            height: 48

            anchors.right: backupBtn.right
            anchors.left: backupBtn.left

            Label {
                text: qsTr("Never show me this again")

                font.pointSize: JamiTheme.textFontSize
            }

            Switch {
                id: passwordSwitch
                Layout.alignment: Qt.AlignRight

                onToggled: {
                    neverShowAgainBoxClicked(checked)
                }
            }
        }

        MaterialButton {
            id: backupBtn
            text: qsTr("BACKUP ACCOUNT")
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: {
                exportBtn_Dialog.open()
                leavePage()
            }
        }

        MaterialButton {
            text: qsTr("SKIP")
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed
            outlined: true

            onClicked: {
                leavePage()
            }
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
