/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0

import "../../commoncomponents"

ItemDelegate {
    id: deviceItemDelegate

    property string deviceName : ""
    property string deviceId : ""
    property bool isCurrent : false

    property bool editable : false

    signal btnRemoveDeviceClicked

    function btnEditDeviceEnter() {
        btnEditDevice.enterBtn()
    }

    function btnEditDeviceExit() {
        btnEditDevice.exitBtn()
    }

    function btnEditPress() {
        btnEditDevice.pressBtn()
    }

    function btnEditRelease() {
        btnEditDevice.releaseBtn()
    }

    function toggleEditable() {
        editable = !editable
        if (editable) {
           SettingsAdapter.setDeviceName(elidedTextDeviceName.text)
        }
    }

    highlighted: ListView.isCurrentItem

    RowLayout {
        id: layoutDeviceItemDelegate
        anchors.fill: parent
        spacing: 8
        Image {
            Layout.leftMargin: 16
            Layout.alignment: Qt.AlignVCenter

            Layout.minimumWidth: 24
            Layout.preferredWidth: 24
            Layout.maximumWidth: 24

            Layout.minimumHeight: 24
            Layout.preferredHeight: 24
            Layout.maximumHeight: 24
            source: "qrc:/images/icons/baseline-desktop_windows-24px.svg"
        }

        ColumnLayout {
            id: col
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: 16

            RowLayout {
                Layout.fillWidth: true
                spacing: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                InfoLineEdit {
                    id: editDeviceName

                    Layout.preferredWidth: deviceItemDelegate.width - 112
                    Layout.maximumWidth: deviceItemDelegate.width - 112
                    Layout.minimumWidth: deviceItemDelegate.width - 112
                    fieldLayoutWidth: deviceItemDelegate.width - 112

                    Layout.minimumHeight: 24
                    Layout.preferredHeight: 24
                    Layout.maximumHeight: 24

                    fieldLayoutHeight: 24

                    Layout.alignment: Qt.AlignLeft
                    font.pointSize: JamiTheme.textFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    readOnly: !editable

                    backgroundColor: "white"

                    text: elidedTextDeviceName.elidedText
                }

                TextMetrics {
                    id: elidedTextDeviceName

                    elide: Text.ElideRight
                    elideWidth: deviceItemDelegate.width - 112

                    text: deviceName
                }
            }

            ElidedTextLabel {
                id: labelDeviceId

                Layout.fillWidth: true
                Layout.leftMargin: 12

                Layout.minimumHeight: 24
                Layout.preferredHeight: 24
                Layout.maximumHeight: 24

                maxWidth: deviceItemDelegate.width - 112
                eText: deviceId === "" ? qsTr("Device Id") : deviceId
            }

        }

        HoverableRadiusButton {
            id: btnEditDevice

            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.rightMargin: 8

            Layout.minimumWidth: 32
            Layout.preferredWidth: 32
            Layout.maximumWidth: 32

            Layout.minimumHeight: 32
            Layout.preferredHeight: 32
            Layout.maximumHeight: 32

            buttonImageHeight: height - 8
            buttonImageWidth: width - 8

            radius: height / 2
            width: 25
            height: 25

            backgroundColor: "transparent"

            source: {
                if(isCurrent) {
                    var path = editable ?
                        "qrc:/images/icons/round-save_alt-24px.svg" :
                        "qrc:/images/icons/round-edit-24px.svg"
                    return path
                } else {
                    return "qrc:/images/icons/round-remove_circle-24px.svg"
                }
            }

            ToolTip.visible: hovered
            ToolTip.text: {
                if(isCurrent) {
                    if (editable) {
                        return qsTr("Edit Device Name")
                    } else {
                        return qsTr("Save new device name")
                    }
                } else {
                    return qsTr("Unlink Device From Account")
                }
            }

            onClicked: {
                if(isCurrent) {
                    toggleEditable()
                } else {
                    btnRemoveDeviceClicked()
                }
            }
        }
    }
}
