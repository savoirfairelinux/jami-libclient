/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ItemDelegate {
    id: root

    property string handlerName : ""
    property string handlerId: ""
    property string handlerIcon: ""
    property bool isLoaded: false
    property string pluginId: ""

    signal btnLoadHandlerToggled
    signal openPreferences

    RowLayout{
        anchors.fill: parent

        Label {
            Layout.leftMargin: 8
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            width: 30

            background: Rectangle{
                color: "transparent"
                Image {
                    anchors.centerIn: parent
                    source: "file:" + handlerIcon
                    width: 30
                    height: 30
                    mipmap: true
                }
            }
        }

        Label {
            id: labelDeviceId
            Layout.leftMargin: 8
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            color: JamiTheme.textColor

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true
            text: handlerName === "" ? handlerId : handlerName
        }

        Switch {
            id: loadSwitch
            property bool isHovering: false

            Layout.rightMargin: 8
            Layout.alignment: Qt.AlignVCenter

            width: 30
            height: 30

            ToolTip.visible: hovered
            ToolTip.text: {
                return qsTr("On/Off")
            }

            checked: isLoaded
            onClicked: {
                btnLoadHandlerToggled()
            }

            background: Rectangle {
                id: switchBackground

                color: "transparent"
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

        PushButton {
            id: btnPreferencesPluginHandler

            Layout.alignment: Qt.AlingVCenter | Qt.AlignRight
            Layout.rightMargin: 8

            source: JamiResources.round_settings_24dp_svg
            normalColor: JamiTheme.primaryBackgroundColor
            imageColor: JamiTheme.textColor
            toolTipText: qsTr(pluginId)

            onClicked: openPreferences()
        }
    }
}
