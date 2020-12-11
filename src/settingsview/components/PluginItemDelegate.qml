/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ItemDelegate {
    id: root

    property string pluginName : ""
    property string pluginId: ""
    property string pluginIcon: ""
    property bool isLoaded: false

    signal btnLoadPluginToggled

    function btnPreferencesPluginClicked() {
        pluginListPreferencesView.pluginName = pluginName
        pluginListPreferencesView.pluginIcon = pluginIcon
        pluginListPreferencesView.pluginId = pluginId
        pluginListPreferencesView.isLoaded = isLoaded
        if (!pluginListPreferencesView.visible) {
            pluginListPreferencesView.visible = !pluginListPreferencesView.visible
            root.height += pluginListPreferencesView.childrenRect.height
        } else {
            root.height -= pluginListPreferencesView.childrenRect.height
            pluginListPreferencesView.visible = !pluginListPreferencesView.visible
        }
    }

    Connections {
        target: enabledplugin

        function onHidePreferences() {
            root.height = 50
            pluginListPreferencesView.visible = false
        }
    }

    ColumnLayout {
        anchors.fill: parent
        Layout.preferredHeight: childrenRect.height

        RowLayout {
            Layout.fillWidth: true

            Label {
                id: pluginImage
                Layout.leftMargin: 8
                Layout.alignment: Qt.AlignLeft | Qt.AlingVCenter
                width: 30

                background: Rectangle {
                    color: "transparent"
                    Image {
                        anchors.centerIn: parent
                        source: "file:" + pluginIcon
                        sourceSize: Qt.size(256, 256)
                        mipmap: true
                        width: 32
                        height: 32
                    }
                }
            }

            Label {
                id: labelDeviceId
                Layout.fillWidth: true
                Layout.leftMargin: 8
                color: JamiTheme.textColor

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                text: pluginName === "" ? pluginId : pluginName
            }

            Switch {
                id: loadSwitch
                property bool isHovering: false
                Layout.rightMargin: 8
                width: 20

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Load/Unload")

                checked: isLoaded
                onClicked: {
                    btnLoadPluginToggled()
                    pluginListPreferencesView.isLoaded = root.isLoaded
                }

                background: Rectangle {
                    id: switchBackground

                    color: "transparent"
                    MouseArea {
                        id: btnMouseArea
                        hoverEnabled: true
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
                id: btnPreferencesPlugin

                Layout.alignment: Qt.AlingVCenter | Qt.AlignRight
                Layout.rightMargin: 8

                source: "qrc:/images/icons/round-settings-24px.svg"
                normalColor: JamiTheme.primaryBackgroundColor
                imageColor: JamiTheme.textColor
                toolTipText: JamiStrings.showHidePrefs

                onClicked: btnPreferencesPluginClicked()
            }
        }

        PluginListPreferencesView {
            id: pluginListPreferencesView

            Layout.topMargin: 10
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
            Layout.bottomMargin: JamiTheme.preferredMarginSize
            Layout.minimumHeight: 1
            Layout.preferredHeight: childrenRect.height
        }
    }
}
