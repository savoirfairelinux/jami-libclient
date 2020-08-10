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
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import "../../commoncomponents"

Rectangle {
    id: pluginSettingsRect

    function populatePluginSettings(){
        // settings
        enabledplugin.checked = ClientWrapper.pluginModel.getPluginsEnabled()
        pluginListSettingsView.visible = enabledplugin.checked
        if (pluginListSettingsView.visible) {
            pluginListSettingsView.updatePluginListDisplayed()
        }
    }

    function slotSetPluginEnabled(state){
        ClientWrapper.pluginModel.setPluginsEnabled(state)
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    signal backArrowClicked

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        width: parent.width
        height: parent.height

        RowLayout {

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: 16
            Layout.fillWidth: true
            Layout.maximumHeight: 64
            Layout.minimumHeight: 64
            Layout.preferredHeight: 64

            HoverableButton {

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30

                radius: 30
                source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                backgroundColor: "white"
                onExitColor: "white"

                visible: mainViewWindow.sidePanelHidden

                onClicked: {
                    backArrowClicked()
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.minimumHeight: 25
                Layout.preferredHeight: 25
                Layout.maximumHeight: 25

                text: qsTr("Plugin")

                font.pointSize: 15
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }

        ScrollView {
            id: pluginScrollView
            Layout.fillHeight: true
            Layout.fillWidth: true

            width: parent.width
            height: parent.height
            focus: true

            clip: true

            ColumnLayout {
                id: pluginViewLayout
                Layout.fillHeight: true
                Layout.fillWidth: true

                ToggleSwitch {
                    id: enabledplugin

                    Layout.topMargin: 15
                    Layout.leftMargin: 36

                    labelText: "Enable"
                    fontPointSize: 13

                    onSwitchToggled: {
                        slotSetPluginEnabled(checked)

                        pluginListSettingsView.visible = checked
                        if (!checked) {
                            pluginListPreferencesView.visible = checked
                            ClientWrapper.pluginModel.toggleCallMediaHandler("",true);
                        }
                        if (pluginListSettingsView.visible) {
                            pluginListSettingsView.updatePluginListDisplayed()
                        }
                    }
                }
                ColumnLayout {
                    spacing: 6
                    Layout.fillHeight: true
                    width:380
                    height:100

                    // instantiate plugin list setting page
                    PluginListSettingsView {
                        id: pluginListSettingsView

                        width:380
                        height:265
                        Layout.leftMargin: 35
                        Layout.topMargin: 15
                        Layout.alignment: Qt.AlignHCenter

                        pluginListPreferencesView: pluginListPreferencesView

                        onScrollView:{ }
                    }

                    PluginListPreferencesView {
                        id: pluginListPreferencesView

                        width:380
                        Layout.minimumHeight: 175
                        Layout.preferredHeight: height
                        Layout.maximumHeight: 1000
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: 55

                        onUpdatePluginList:{
                            pluginListSettingsView.updateAndShowPluginsSlot()
                        }
                    }
                }
            }
        }
    }
}
