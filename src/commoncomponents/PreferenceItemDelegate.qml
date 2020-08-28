/**
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

import QtQuick 2.14
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import Qt.labs.platform 1.1
import QtQuick.Dialogs 1.3
import net.jami.Models 1.0

import "../commoncomponents"

ItemDelegate {
    id: root

    enum Type {
        LIST,
        USERLIST,
        DEFAULT
    }

    property string preferenceName: ""
    property string preferenceSummary: ""
    property int preferenceType: -1
    property string preferenceCurrentValue: ""
    property string preferenceNewValue: ""
    property string pluginId: ""
    property PluginListPreferenceModel pluginListPreferenceModel

    signal btnPreferenceClicked
    signal preferenceAdded

    function getNewPreferenceValueSlot(index){
        pluginListPreferenceModel.idx = index
        preferenceNewValue = pluginListPreferenceModel.preferenceNewValue
        switch (preferenceType){
            case PreferenceItemDelegate.LIST:
                btnPreferenceClicked()
                break
            case PreferenceItemDelegate.USERLIST:
                if(index == 0){
                    preferenceFilePathDialog.pluginListPreferenceModel = pluginListPreferenceModel
                    preferenceFilePathDialog.title = qsTr("Select An Image to " + preferenceName)
                    preferenceFilePathDialog.nameFilters = [qsTr("PNG Files") + " (*.png)", qsTr(
                "All files") + " (*)"]
                    preferenceFilePathDialog.preferenceKey = pluginListPreferenceModel.preferenceKey
                    preferenceFilePathDialog.open()
                }
                else
                    btnPreferenceClicked()
                break
            default:
                break
        }
    }

    FileDialog {
        id: preferenceFilePathDialog

        property string preferenceKey: ""
        property PluginListPreferenceModel pluginListPreferenceModel

        title: qsTr("Please choose a file")
        folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

        onRejected: preferenceAdded()

        onAccepted: {
            var url = ClientWrapper.utilsAdaptor.getAbsPath(fileUrl.toString())
            ClientWrapper.pluginModel.addValueToPreference(pluginId, preferenceKey, url)
            pluginListPreferenceModel.populateLists()
            pluginListPreferenceModel.getCurrentSettingIndex()
            preferenceAdded()
        }
    }

    RowLayout{
        anchors.fill: parent

        Label{
            visible: preferenceType === PreferenceItemDelegate.DEFAULT
            Layout.fillWidth: true
            Layout.alignment: Qt.AlingVCenter | Qt.AligntLeft
            Layout.leftMargin: 8

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true
            font.bold: true
            text: preferenceName
        }

        HoverableRadiusButton{
            id: btnPreference
            visible: preferenceType === PreferenceItemDelegate.DEFAULT
            backgroundColor: "white"

            Layout.alignment: Qt.AlignRight | Qt.AlingVCenter
            Layout.rightMargin: 7
            Layout.preferredWidth: 30
            Layout.preferredHeight: 30

            buttonImageHeight: 20
            buttonImageWidth: 20

            source:{
                return "qrc:/images/icons/round-settings-24px.svg"
            }

            ToolTip.visible: hovered
            ToolTip.text: {
                return qsTr("Edit preference")
            }

            onClicked: {
                btnPreferenceClicked()
            }
        }

        Label {
            visible: preferenceType === PreferenceItemDelegate.LIST
            Layout.preferredWidth: root.width / 2
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: 8

            text: preferenceName
            font.pointSize: JamiTheme.settingsFontSize
            ToolTip.visible: hovered
            ToolTip.text: preferenceSummary
        }


        SettingParaCombobox {
            id: listPreferenceComboBox
            visible: preferenceType === PreferenceItemDelegate.LIST
            Layout.preferredWidth: root.width / 2 - 8
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 8

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            model: pluginListPreferenceModel
            currentIndex: pluginListPreferenceModel.getCurrentSettingIndex()
            textRole: qsTr("PreferenceValue")
            tooltipText: qsTr("Choose the preference")
            onActivated: {
                getNewPreferenceValueSlot(index)
            }
        }

        Label {
            visible: preferenceType === PreferenceItemDelegate.USERLIST
            Layout.preferredWidth: root.width / 2
            Layout.leftMargin: 8
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            text: preferenceName
            font.pointSize: JamiTheme.settingsFontSize
            ToolTip.visible: hovered
            ToolTip.text: preferenceSummary
        }


        SettingParaCombobox {
            id: userListPreferenceComboBox
            visible: preferenceType === PreferenceItemDelegate.USERLIST
            Layout.preferredWidth: root.width / 2 - 8
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 8

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            model: pluginListPreferenceModel
            currentIndex: pluginListPreferenceModel.getCurrentSettingIndex()
            textRole: qsTr("PreferenceValue")
            tooltipText: qsTr("Choose the preference")
            onActivated: {
                getNewPreferenceValueSlot(index)
            }
        }
    }
}
