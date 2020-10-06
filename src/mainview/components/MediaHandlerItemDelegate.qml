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
    id: root

    property string mediaHandlerName : ""
    property string mediaHandlerId: ""
    property string mediaHandlerIcon: ""
    property bool isLoaded: false
    property string pluginId: ""

    signal btnLoadMediaHandlerToggled
    signal openPreferences

    RowLayout{
        anchors.fill: parent

        Label {
            Layout.leftMargin: 8
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            width: 30

            background: Rectangle{
                Image {
                    anchors.centerIn: parent
                    source: "file:" + mediaHandlerIcon
                    width: 30
                    height: 30
                }
            }
        }

        Label {
            id: labelDeviceId
            Layout.leftMargin: 8
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true
            text: mediaHandlerName === "" ? mediaHandlerId : mediaHandlerName
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
                btnLoadMediaHandlerToggled()
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

        PushButton {
            id: btnPreferencesMediaHandler

            Layout.alignment: Qt.AlingVCenter | Qt.AlignRight
            Layout.rightMargin: 8

            normalColor: JamiTheme.primaryBackgroundColor
            source: "qrc:/images/icons/round-settings-24px.svg"

            toolTipText: qsTr(pluginId)

            onClicked: openPreferences()
        }
    }
}
