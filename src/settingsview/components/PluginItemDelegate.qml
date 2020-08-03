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
    id: pluginItemDelegate

    property string pluginName : ""
    property string pluginId: ""
    property string pluginIcon: ""
    property bool isLoaded: false

    signal btnLoadPluginToggled
    signal btnPreferencesPluginClicked

    highlighted: ListView.isCurrentItem

    RowLayout{
        anchors.fill: parent

        Label{
            Layout.leftMargin: 7
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
                    source: "file:"+pluginIcon
                }
            }
        }

        ColumnLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.leftMargin: 7
            Layout.topMargin: 7
            Layout.bottomMargin: 7

            RowLayout{

                Layout.minimumHeight: 30

                Label{
                    id: labelDeviceId

                    Layout.minimumHeight: 20

                    font.pointSize: 10
                    font.kerning: true
                    text: pluginName === "" ? pluginId : pluginName
                }

                Item{
                    Layout.fillWidth: true

                    Layout.minimumWidth: 0
                    Layout.minimumHeight: 20
                }
            }
        }

        Switch {
            id: loadSwitch
            property bool isHovering: false

            Layout.bottomMargin: 7
            Layout.rightMargin: 15

            Layout.maximumWidth: 30
            Layout.preferredWidth: 30
            Layout.minimumWidth: 30

            Layout.minimumHeight: 30
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30

            ToolTip.visible: isHovering
            ToolTip.text: {
                return qsTr("Load/Unload")
            }

            checked: isLoaded
            onClicked: {
                btnLoadPluginToggled()
            }

            background: Rectangle {
                id: switchBackground
                MouseArea {
                    id: btnMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                    }
                    onReleased: {
                        loadSwitch.clicked()
                    }
                    onEntered: {
                        loadSwitch.isHovering = true
                    }
                    onExited: {
                        loadSwitch.isHovering = false
                    }
                }
            }
        }

        HoverableRadiusButton{
            id: btnPreferencesPlugin

            Layout.bottomMargin: 7
            Layout.rightMargin: 7
            Layout.alignment: Qt.AlignRight

            Layout.minimumWidth: 30
            Layout.preferredWidth: 30
            Layout.maximumWidth: 30

            Layout.minimumHeight: 30
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30

            buttonImageHeight: height
            buttonImageWidth: height

            source:{
                return "qrc:/images/icons/round-settings-24px.svg"
            }

            ToolTip.visible: isHovering
            ToolTip.text: {
                return qsTr("Edit preferences")
            }

            onClicked: {
                btnPreferencesPluginClicked()
            }
        }
    }
}
