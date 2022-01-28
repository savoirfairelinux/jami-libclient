/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos  <aline.gondimsantos@savoirfairelinux.com>
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
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property int contentWidth: pluginSettingsColumnLayout.width
    property int preferredHeight: pluginSettingsColumnLayout.implicitHeight

    function populatePluginSettings() {
        enabledplugin.checked = PluginModel.getPluginsEnabled()
        pluginListSettingsView.visible = enabledplugin.checked
    }

    function slotSetPluginEnabled(state) {
        PluginModel.setPluginsEnabled(state)
    }

    ColumnLayout {
        id: pluginSettingsColumnLayout

        anchors.horizontalCenter: root.horizontalCenter

        width: Math.min(JamiTheme.maximumWidthSettingsView, root.width)

        ToggleSwitch {
            id: enabledplugin

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            labelText: JamiStrings.enable
            fontPointSize: JamiTheme.headerFontSize

            onSwitchToggled: {
                slotSetPluginEnabled(checked)

                pluginListSettingsView.visible = checked
                if (!pluginListSettingsView.visible) {
                    PluginModel.toggleCallMediaHandler("", true)
                    pluginListSettingsView.hidePreferences()
                }
            }
        }

        PluginListSettingsView {
            id: pluginListSettingsView

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            pluginListPreferencesView: pluginListPreferencesView

            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.minimumHeight: 0
            Layout.preferredHeight: childrenRect.height
        }

        PluginListPreferencesView {
            id: pluginListPreferencesView

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
            Layout.bottomMargin: JamiTheme.preferredMarginSize
            Layout.minimumHeight: 0
            Layout.preferredHeight: childrenRect.height
        }
    }
}
