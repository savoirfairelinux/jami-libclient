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
import net.jami.Adapters 1.0

import "../../commoncomponents"

ItemDelegate {
    id: root

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
        anchors.fill: root

        Image {
            id: deviceImage
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.alignment: Qt.AlignVCenter

            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            source: "qrc:/images/icons/baseline-desktop_windows-24px.svg"
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize

            InfoLineEdit {
                id: editDeviceName
                implicitWidth: parent.width
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                Layout.alignment: Qt.AlignLeft
                font.pointSize: JamiTheme.textFontSize
                font.kerning: true

                wrapMode: Text.NoWrap
                readOnly: !editable
                backgroundColor: "white"
                text: elidedTextDeviceName.elidedText
            }

            TextMetrics {
                id: elidedTextDeviceName

                elide: Text.ElideRight
                elideWidth: root.width - btnEditDevice.width - deviceImage.width - 8
                text: deviceName
            }

            ElidedTextLabel {
                id: labelDeviceId

                Layout.preferredHeight: 24
                Layout.leftMargin: 8

                maxWidth: root.width - btnEditDevice.width - deviceImage.width
                eText: deviceId === "" ? qsTr("Device Id") : deviceId
            }
        }

        PushButton {
            id: btnEditDevice

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 8
            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

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

            toolTipText: {
                if(isCurrent) {
                    if (editable) {
                        return JamiStrings.editDeviceName
                    } else {
                        return qsTr("Save new device name")
                    }
                } else {
                    return JamiStrings.unlinkDevice
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
