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
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import "../../commoncomponents"

Rectangle {
    id: pluginListPreferencesViewRect

    enum Type {
        LIST,
        DEFAULT
    }

    signal updatePluginList

    property string pluginName: ""
    property string pluginIcon: ""
    property string pluginId: ""
    property bool isLoaded: false
    property int size: 0

    visible: false

    function updatePreferenceListDisplayed(show){
        // settings
        getSize(pluginId, show)
        preferenceItemListModel.pluginId = pluginId
        preferenceItemListModel.reset()
    }

    function resetPluginSlot(){
        resetPluginMessageBox.open()
    }

    function resetPlugin(){
        ClientWrapper.pluginModel.resetPluginPreferencesValues(pluginId, isLoaded)
        updatePluginList()
    }

    function uninstallPluginSlot(){
        uninstallPluginMessageBox.open()
    }

    function uninstallPlugin(){
        ClientWrapper.pluginModel.uninstallPlugin(pluginId)
        updatePluginList()
    }

    function getSize(pluginId, show){
        preferenceItemListModel.pluginId = pluginId
        size = 50 * preferenceItemListModel.preferencesCount
        if (show) {
            height = 200 + size
            pluginPreferenceView.height = size
        } else {
            height = 25
        }
    }

    function editPreferenceSlot(preferenceType, preferenceName, preferenceEntryValues){
        switch (preferenceType){
            case PluginListPreferencesView.LIST:
                console.log("LIST")
                editListMessageBox.preferenceName = preferenceName
                editListMessageBox.preferenceEntryValues =  preferenceEntryValues
                editListMessageBox.open()
                break
            case PluginListPreferencesView.DEFAULT:
                console.log("Unrecognizable Type")
                break
            default:
                console.log("Unrecognizable Type")
                break
        }
    }

    function setPreference(pluginId, preferenceKey, preferenceNewValue)
    {
        ClientWrapper.pluginModel.setPluginPreferences(pluginId, preferenceKey, preferenceNewValue, isLoaded)
        preferenceItemListModel.reset()
    }

    MessageBox{
        id: uninstallPluginMessageBox

        title:qsTr("Uninstall plugin")
        text :qsTr("Are you sure you wish to uninstall " + pluginName + " ?")
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onYes: {
            accepted()
        }

        onNo:{
            rejected()
        }

        onDiscard: {
            rejected()
        }

        onAccepted: {
            uninstallPlugin()
            pluginListPreferencesViewRect.visible = false
        }

        onRejected: {}
    }

    MessageBox{
        id: resetPluginMessageBox

        title:qsTr("Reset preferences")
        text :qsTr("Are you sure you wish to reset "+ pluginName + " preferences?")

        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onYes: {
            accepted()
        }

        onNo:{
            rejected()
        }

        onDiscard: {
            rejected()
        }

        onAccepted: {
            resetPlugin()
        }

        onRejected: {}
    }

    MessageBox{
        id: editListMessageBox

        property string preferenceName: ""
        property var preferenceEntryValues: []
        
        title:qsTr("Edit " + preferenceName)
        text :qsTr(preferenceName + " options: " + preferenceEntryValues)

        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onYes: {
            accepted()
        }

        onNo:{
            rejected()
        }

        onDiscard: {
            rejected()
        }

        onAccepted: {
            // setPreference(pluginId, preferenceItemDelegate.preferenceKey, preferenceItemDelegate.preferenceNewValue)
        }

        onRejected: {}
    }

    PreferenceItemListModel {
        id: preferenceItemListModel
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    ColumnLayout {
        spacing: 6
        Layout.fillHeight: true
        Layout.maximumWidth: 580
        Layout.preferredWidth: 580
        Layout.minimumWidth: 580

        Label{
            Layout.alignment: Qt.AlignHCenter

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

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
            Layout.fillWidth: true
            Layout.minimumHeight: 25
            Layout.preferredHeight: 25
            Layout.maximumHeight: 25

            text: qsTr(pluginName + "\npreferences")
            font.pointSize: 13
            font.kerning: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        RowLayout {
            spacing: 6
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.maximumHeight: 30
            Layout.preferredHeight: 30
            Layout.minimumHeight: 30

            HoverableRadiusButton {
                id: resetButton

                Layout.maximumWidth: 157
                Layout.preferredWidth: 157
                Layout.minimumWidth: 157

                Layout.fillHeight: true

                radius: height / 2

                icon.source: "qrc:/images/icons/settings_backup_restore-black-18dp.svg"
                icon.height: 24
                icon.width: 24

                text: qsTr("Reset")
                fontPointSize: 10
                font.kerning: true

                onClicked: {
                    resetPluginSlot()
                }
            }

            HoverableRadiusButton {
                id: uninstallButton

                Layout.maximumWidth: 157
                Layout.preferredWidth: 157
                Layout.minimumWidth: 157

                Layout.fillHeight: true

                radius: height / 2

                icon.source: "qrc:/images/icons/ic_delete_black_18dp_2x.png"
                icon.height: 24
                icon.width: 24

                text: qsTr("Uninstall")
                fontPointSize: 10
                font.kerning: true

                onClicked: {
                    uninstallPluginSlot()
                }
            }
        }

        ListViewJami {
            id: pluginPreferenceView

            Layout.minimumWidth: 320
            Layout.preferredWidth: 320
            Layout.maximumWidth: 320

            Layout.minimumHeight: 0
            Layout.preferredHeight: height
            Layout.maximumHeight: 1000

            model: preferenceItemListModel

            delegate: PreferenceItemDelegate{
                id: preferenceItemDelegate

                width: pluginPreferenceView.width
                height: 50

                preferenceKey : PreferenceKey
                preferenceName: PreferenceName
                preferenceSummary: PreferenceSummary
                preferenceType: PreferenceType
                preferenceDefaultValue: PreferenceDefaultValue
                preferenceEntries: PreferenceEntries
                preferenceEntryValues: PreferenceEntryValues

                onClicked: {
                    pluginPreferenceView.currentIndex = index
                }
                onBtnPreferenceClicked: {
                    console.log("edit preference ", preferenceName)
                    console.log("preference type ", preferenceType)
                    console.log("preference entry values ", preferenceEntryValues.length)
                    editPreferenceSlot(preferenceType, preferenceName, preferenceEntryValues)
                }
            }
        }
    }
}
