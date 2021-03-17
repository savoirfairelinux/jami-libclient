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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import Qt.labs.platform 1.1
import QtQuick.Dialogs 1.3

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../commoncomponents"

ItemDelegate {
    id: root

    enum Type {
        LIST,
        PATH,
        EDITTEXT,
        SWITCH,
        DEFAULT
    }

    property string preferenceName: ""
    property string preferenceSummary: ""
    property string preferenceKey: ""
    property int preferenceType: -1
    property string preferenceCurrentValue: ""
    property string preferenceNewValue: ""
    property string pluginId: ""
    property string currentPath: ""
    property bool isImage: false
    property var fileFilters: []
    property PluginListPreferenceModel pluginListPreferenceModel

    signal btnPreferenceClicked

    function getNewPreferenceValueSlot(index) {
        switch (preferenceType) {
            case PreferenceItemDelegate.LIST:
                pluginListPreferenceModel.idx = index
                preferenceNewValue = pluginListPreferenceModel.preferenceNewValue
                btnPreferenceClicked()
                break
            case PreferenceItemDelegate.PATH:
                if (index === 0) {
                    preferenceFilePathDialog.title = qsTr("Select An Image to " + preferenceName)
                    preferenceFilePathDialog.nameFilters = fileFilters
                    preferenceFilePathDialog.selectedNameFilter.index = fileFilters.length - 1
                    preferenceFilePathDialog.open()
                }
                else
                    btnPreferenceClicked()
                break
            case PreferenceItemDelegate.EDITTEXT:
                preferenceNewValue = editTextPreference.text
                btnPreferenceClicked()
                break
            case PreferenceItemDelegate.SWITCH:
                preferenceNewValue = index ? "1" : "0"
                btnPreferenceClicked()
                break
            default:
                break
        }
    }

    JamiFileDialog {
        id: preferenceFilePathDialog

        title: JamiStrings.selectFile
        folder: "file:///" + currentPath

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())
            preferenceNewValue = url
            btnPreferenceClicked()
        }
    }

    RowLayout{
        anchors.fill: parent

        Text {
            id: prefLlabel
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: 8

            text: preferenceName
            color: JamiTheme.textColor
            elide: Text.ElideRight
            font.pointSize: JamiTheme.settingsFontSize
            ToolTip.visible: hovered
            ToolTip.text: preferenceSummary
        }

        PushButton {
            id: btnPreferenceDefault

            visible: preferenceType === PreferenceItemDelegate.DEFAULT
            normalColor: JamiTheme.primaryBackgroundColor

            Layout.alignment: Qt.AlignRight | Qt.AlingVCenter
            Layout.rightMargin: 8
            Layout.preferredWidth: preferredSize
            Layout.preferredHeight: preferredSize
            imageColor: JamiTheme.textColor

            source: "qrc:/images/icons/round-settings-24px.svg"

            toolTipText: qsTr("Edit preference")
        }

        Switch {
            id: btnPreferenceSwitch

            visible: preferenceType === PreferenceItemDelegate.SWITCH
            Layout.alignment: Qt.AlignRight | Qt.AlingVCenter
            Layout.rightMargin: 16
            Layout.preferredHeight: 30
            Layout.preferredWidth: 30
            checked: preferenceCurrentValue === "1"

            onToggled: getNewPreferenceValueSlot(checked)
        }

        SettingParaCombobox {
            id: listPreferenceComboBox

            visible: preferenceType === PreferenceItemDelegate.LIST
            Layout.preferredWidth: root.width / 2 - 8
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 4

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            model: pluginListPreferenceModel
            currentIndex: pluginListPreferenceModel.getCurrentSettingIndex()
            textRole: "PreferenceValue"
            tooltipText: JamiStrings.select
            onActivated: getNewPreferenceValueSlot(index)
        }

        MaterialButton {
            id: pathPreferenceButton

            visible: preferenceType === PreferenceItemDelegate.PATH
            width: root.width / 2 - 8
            Layout.preferredWidth: width
            Layout.preferredHeight: 30
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 4

            text: UtilsAdapter.fileName(preferenceCurrentValue)
            toolTipText: JamiStrings.chooseImageFile
            source: "qrc:/images/icons/round-folder-24px.svg"
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: getNewPreferenceValueSlot(0)
        }

        MaterialLineEdit {
            id: editTextPreference

            visible: preferenceType === PreferenceItemDelegate.EDITTEXT
            width: root.width / 2 - 8
            Layout.preferredWidth: root.width / 2 - 8
            Layout.preferredHeight: 30
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 4

            selectByMouse: true
            text: preferenceCurrentValue

            font.pointSize: JamiTheme.settingsFontSize
            padding: 8
            wrapMode: Text.NoWrap

            onEditingFinished: getNewPreferenceValueSlot(0)
        }
    }
}
