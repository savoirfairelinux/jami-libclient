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
    property string deviceId: ""
    property bool isCurrent : false

    property bool editable : false

    signal btnRemoveDeviceClicked

    function btnEditDeviceEnter(){
        btnEditDevice.enterBtn()
    }

    function btnEditDeviceExit(){
        btnEditDevice.exitBtn()
    }

    function btnEditPress(){
        btnEditDevice.pressBtn()
    }

    function btnEditRelease(){
        btnEditDevice.releaseBtn()
    }

    function toggleEditable(){
        editable = !editable
        if(editable){
            ClientWrapper.settingsAdaptor.setDeviceName(editDeviceName.text)
        }
    }

    highlighted: ListView.isCurrentItem

    RowLayout{
        anchors.fill: parent

        spacing: 7

        Label{
            Layout.leftMargin: 7
            Layout.topMargin: 7
            Layout.bottomMargin: 7

            Layout.minimumWidth: 30
            Layout.preferredWidth: 30
            Layout.maximumWidth: 30

            Layout.minimumHeight: 30
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30

            background: Rectangle{
                anchors.fill: parent
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/icons/baseline-desktop_windows-24px.svg"
                }
            }
        }

        ColumnLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.topMargin: 7
            Layout.bottomMargin: 7

            InfoLineEdit{
                id: editDeviceName

                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.maximumWidth: 16777215

                Layout.minimumHeight: 30
                Layout.preferredHeight: 30
                Layout.maximumHeight: 30

                font.pointSize: 8
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                readOnly: !editable

                text: deviceName
            }

            RowLayout{
                Layout.maximumWidth: editDeviceName.fieldLayoutWidth

                Layout.minimumHeight: 30

                Label{
                    id: labelDeviceId

                    //Layout.minimumWidth: 71
                    Layout.minimumHeight: 30

                    font.pointSize: 8
                    font.kerning: true
                    text: deviceId === "" ? qsTr("Device Id") : deviceId
                }

                Item{
                    Layout.fillWidth: true

                    Layout.minimumWidth: 0
                    Layout.minimumHeight: 20
                }

                Label{
                    id: labelThisDevice

                    //Layout.minimumWidth: 80
                    Layout.minimumHeight: 30

                    visible: isCurrent

                    font.pointSize: 8
                    font.kerning: true
                    font.italic: true
                    color: "green"
                    text:  qsTr("this device")
                }
            }
        }

        HoverableRadiusButton{
            id: btnEditDevice

            Layout.topMargin: 7
            Layout.bottomMargin: 7

            Layout.minimumWidth: 30
            Layout.preferredWidth: 30
            Layout.maximumWidth: 30

            Layout.minimumHeight: 30
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30

            buttonImageHeight: height
            buttonImageWidth: height

            source:{
                if(isCurrent) {
                    var path = editable ? "qrc:/images/icons/round-edit-24px.svg" : "qrc:/images/icons/round-save_alt-24px.svg"
                    return path
                } else {
                    return "qrc:/images/icons/round-remove_circle-24px.svg"
                }
            }

            ToolTip.visible: isHovering
            ToolTip.text: {
                if(isCurrent) {
                    if(editable){
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

        Item{
            Layout.rightMargin: 7

            Layout.minimumWidth: 8
            Layout.preferredWidth: 8
            Layout.maximumWidth: 8

            Layout.minimumHeight: 20
        }
    }
}
