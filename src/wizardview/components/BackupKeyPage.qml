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

    color: JamiTheme.backgroundColor

    ColumnLayout {
        spacing: layoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            spacing: layoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: backupBtn.width

            Label {
                text: qsTr("Backup your account!")
                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                Layout.alignment: Qt.AlignRight

                text: qsTr("Recommended")
                color: "white"
                padding: 8

                background: Rectangle {
                    color: "#aed581"
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        Label {
            property int preferredHeight: 0

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: backupBtn.width
            Layout.preferredHeight: preferredHeight

            text: qsTr("This account only exists on this device. " +
                       "If you lost your device or uninstall the application, " +
                       "your account will be deleted. " +
                       "You can backup your account now or later.")
            wrapMode: Text.WordWrap
            font.pointSize: JamiTheme.textFontSize

            onFontChanged: {
                var boundingRect = JamiQmlUtils.getTextBoundingRect(font, text)
                preferredHeight = (boundingRect.width / backupBtn.preferredWidth)
                        * boundingRect.height
            }
        }

        RowLayout {
            spacing: layoutSpacing

            Layout.alignment: Qt.AlignCenter

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

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

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
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

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
}
