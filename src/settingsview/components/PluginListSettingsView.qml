/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Aline Gondim Sanots  <aline.gondimsantos@savoirfairelinux.com>
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
import net.jami.Adapters 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property PluginListPreferencesView pluginListPreferencesView

    Connections {
        target: pluginListPreferencesView

        function onUninstalled() {
            pluginListView.model = PluginAdapter.getPluginSelectableModel()
        }
    }

    visible: false

    function openPluginFileSlot() {
        pluginPathDialog.open()
    }

    function loadPluginSlot(pluginId, isLoaded) {
        var loaded = false
        if (isLoaded)
            PluginModel.unloadPlugin(pluginId)
        else
            loaded = PluginModel.loadPlugin(pluginId)
        if (pluginListPreferencesView.pluginId === pluginId)
            pluginListPreferencesView.isLoaded = loaded
    }

    function openPreferencesPluginSlot(pluginName, pluginIcon, pluginId, isLoaded) {
        if (pluginListPreferencesView.pluginId == pluginId || pluginListPreferencesView.pluginId == "")
            pluginListPreferencesView.visible = !pluginListPreferencesView.visible

        if (!pluginListPreferencesView.visible) {
            pluginListPreferencesView.pluginId = ""
        } else {
            pluginListPreferencesView.pluginName = pluginName
            pluginListPreferencesView.pluginIcon = pluginIcon
            pluginListPreferencesView.pluginId = pluginId
            pluginListPreferencesView.isLoaded = isLoaded
        }
    }

    function hidePreferences() {
        pluginListPreferencesView.pluginId = ""
        pluginListPreferencesView.visible = false
    }

    JamiFileDialog {
        id: pluginPathDialog

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectPluginInstall
        folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

        nameFilters: [qsTr("Plugin Files") + " (*.jpl)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())
            PluginModel.installPlugin(url, true)
            pluginListView.model = PluginAdapter.getPluginSelectableModel()
        }
    }

    ColumnLayout {
        id: pluginListViewLayout
        anchors.left: root.left
        anchors.right: root.right

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: 25

            text: qsTr("Installed plugins")
            font.pointSize: JamiTheme.headerFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        MaterialButton {
            id: installButton

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: JamiTheme.preferredFieldWidth
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.topMargin: JamiTheme.preferredMarginSize / 2

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true
            toolTipText: JamiStrings.addNewPlugin

            source: "qrc:/images/icons/round-add-24px.svg"

            text: JamiStrings.installPlugin

            onClicked: openPluginFileSlot()
        }

        ListView {
            id: pluginListView

            Layout.fillWidth: true
            Layout.minimumHeight: 0
            Layout.preferredHeight: childrenRect.height

            model: PluginAdapter.getPluginSelectableModel()
            interactive: false

            delegate: PluginItemDelegate {
                id: pluginItemDelegate

                width: pluginListView.width
                height: 50

                pluginName: PluginName
                pluginId: PluginId
                pluginIcon: PluginIcon
                isLoaded: IsLoaded

                onBtnLoadPluginToggled: {
                    loadPluginSlot(pluginId, isLoaded)
                    pluginListView.model = PluginAdapter.getPluginSelectableModel()
                }

                onBtnPreferencesPluginClicked: openPreferencesPluginSlot(pluginName, pluginIcon, pluginId, isLoaded)

                background: Rectangle {
                    anchors.fill: parent
                    color: "white"
                }
            }
        }
    }
}
