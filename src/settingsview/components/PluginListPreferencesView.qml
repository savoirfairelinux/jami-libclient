/**
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import "../../commoncomponents"

Rectangle {
    id: root

    enum Type {
        LIST,
        PATH,
        DEFAULT
    }

    property string pluginName: ""
    property string pluginIcon: ""
    property string pluginId: ""
    property bool isLoaded: false

    visible: false

    function resetPluginSlot(){
        resetPluginMessageBox.open()
    }

    function resetPlugin(){
        if (isLoaded){
            ClientWrapper.pluginModel.unloadPlugin(pluginId)
            ClientWrapper.pluginModel.resetPluginPreferencesValues(pluginId)
            ClientWrapper.pluginModel.loadPlugin(pluginId)
        } else {
            ClientWrapper.pluginModel.resetPluginPreferencesValues(pluginId)
        }
        pluginPreferenceView.model = PluginAdapter.getPluginPreferencesModel(pluginId)
    }

    function uninstallPluginSlot(){
        uninstallPluginMessageBox.open()
    }

    function uninstallPlugin(){
        ClientWrapper.pluginModel.uninstallPlugin(pluginId)
    }

    function setPreference(pluginId, preferenceKey, preferenceNewValue)
    {
        if (isLoaded){
            ClientWrapper.pluginModel.unloadPlugin(pluginId)
            ClientWrapper.pluginModel.setPluginPreference(pluginId, preferenceKey, preferenceNewValue)
            ClientWrapper.pluginModel.loadPlugin(pluginId)
        }
        else {
            ClientWrapper.pluginModel.setPluginPreference(pluginId, preferenceKey, preferenceNewValue)
        }
    }

    MessageDialog{
        id: uninstallPluginMessageBox

        title:qsTr("Uninstall plugin")
        text :qsTr("Are you sure you wish to uninstall " + pluginName + " ?")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onAccepted: {
            uninstallPlugin()
            root.visible = false
        }
    }

    MessageDialog{
        id: resetPluginMessageBox

        title:qsTr("Reset preferences")
        text :qsTr("Are you sure you wish to reset "+ pluginName + " preferences?")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onAccepted: resetPlugin()
    }

    ColumnLayout {
        anchors.left: root.left
        anchors.right: root.right

        Label{
            Layout.alignment: Qt.AlignHCenter
            background: Rectangle{
                Image {
                    anchors.centerIn: parent
                    source: "file:"+pluginIcon
                    height: 35
                    width: 35
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10

            text: qsTr(pluginName + "\npreferences")
            font.pointSize: JamiTheme.headerFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        RowLayout {
            Layout.topMargin: 10
            height: 30

            HoverableRadiusButton {
                id: resetButton
                Layout.fillWidth: true

                radius: height / 2

                icon.source: "qrc:/images/icons/settings_backup_restore-black-18dp.svg"
                icon.height: 24
                icon.width: 24

                text: qsTr("  Reset  ")
                fontPointSize: JamiTheme.settingsFontSize
                font.kerning: true

                onClicked: {
                    resetPluginSlot()
                }
            }

            HoverableRadiusButton {
                id: uninstallButton
                Layout.fillWidth: true

                radius: height / 2

                icon.source: "qrc:/images/icons/ic_delete_black_18dp_2x.png"
                icon.height: 24
                icon.width: 24

                text: qsTr("Uninstall")
                fontPointSize: JamiTheme.settingsFontSize
                font.kerning: true

                onClicked: uninstallPluginSlot()
            }
        }

        ListView {
            id: pluginPreferenceView

            Layout.fillWidth: true
            Layout.minimumHeight: 0
            Layout.preferredHeight: childrenRect.height + 30

            model: PluginAdapter.getPluginPreferencesModel(pluginId)

            delegate: PreferenceItemDelegate{
                id: preferenceItemDelegate

                width: pluginPreferenceView.width
                height: childrenRect.height

                preferenceName: PreferenceName
                preferenceSummary: PreferenceSummary
                preferenceType: PreferenceType
                preferenceCurrentValue: PreferenceCurrentValue
                pluginId: PluginId
                currentPath: CurrentPath
                preferenceKey: PreferenceKey
                fileFilters: FileFilters
                isImage: IsImage
                pluginListPreferenceModel: PluginListPreferenceModel{
                    id: pluginListPreferenceModel
                    preferenceKey : PreferenceKey
                    pluginId: PluginId
                }

                onClicked: {
                    pluginPreferenceView.currentIndex = index
                }
                onBtnPreferenceClicked: {
                    setPreference(pluginId, preferenceKey, preferenceNewValue)
                    pluginPreferenceView.model = PluginAdapter.getPluginPreferencesModel(pluginId)
                }
            }
        }
    }
}
