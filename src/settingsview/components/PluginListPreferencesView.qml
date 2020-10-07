/*
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

    signal uninstalled

    function resetPluginSlot() {
        msgDialog.buttonCallBacks = [function () {resetPlugin()}]
        msgDialog.openWithParameters(qsTr("Reset preferences"),
                                     qsTr("Are you sure you wish to reset "+ pluginName +
                                          " preferences?"))
    }

    function resetPlugin() {
        if (isLoaded) {
            PluginModel.unloadPlugin(pluginId)
            PluginModel.resetPluginPreferencesValues(pluginId)
            PluginModel.loadPlugin(pluginId)
        } else {
            PluginModel.resetPluginPreferencesValues(pluginId)
        }
        pluginPreferenceView.model = PluginAdapter.getPluginPreferencesModel(pluginId)
    }

    function uninstallPluginSlot() {
        msgDialog.buttonCallBacks = [function () {
            uninstallPlugin()
            root.visible = false
        }]
        msgDialog.openWithParameters(qsTr("Uninstall plugin"),
                                     qsTr("Are you sure you wish to uninstall " + pluginName + " ?"))
    }

    function uninstallPlugin() {
        PluginModel.uninstallPlugin(pluginId)
        uninstalled()
    }

    function setPreference(pluginId, preferenceKey, preferenceNewValue)
    {
        if (isLoaded) {
            PluginModel.unloadPlugin(pluginId)
            PluginModel.setPluginPreference(pluginId, preferenceKey, preferenceNewValue)
            PluginModel.loadPlugin(pluginId)
        } else
            PluginModel.setPluginPreference(pluginId, preferenceKey, preferenceNewValue)
    }

    SimpleMessageDialog {
        id: msgDialog

        buttonTitles: [qsTr("Ok"), qsTr("Cancel")]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue,
                       SimpleMessageDialog.ButtonStyle.TintedBlack]

        onAccepted: {
            uninstallPlugin()
            root.visible = false
        }
    }

    ColumnLayout {
        anchors.left: root.left
        anchors.right: root.right

        Label{
            Layout.alignment: Qt.AlignHCenter
            background: Rectangle {
                Image {
                    anchors.centerIn: parent
                    source: pluginIcon === "" ? "" : "file:" + pluginIcon
                    sourceSize: Qt.size(256, 256)
                    height: 48
                    width: 48
                    mipmap: true
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 16

            text: qsTr(pluginName + "\npreferences")
            font.pointSize: JamiTheme.headerFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        RowLayout {
            Layout.topMargin: 10
            height: 30

            MaterialButton {
                id: resetButton

                Layout.fillWidth: true
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                color: JamiTheme.buttonTintedBlack
                hoveredColor: JamiTheme.buttonTintedBlackHovered
                pressedColor: JamiTheme.buttonTintedBlackPressed
                outlined: true

                source: "qrc:/images/icons/settings_backup_restore-24px.svg"

                text: JamiStrings.reset

                onClicked: resetPluginSlot()
            }

            MaterialButton {
                id: uninstallButton

                Layout.fillWidth: true
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                color: JamiTheme.buttonTintedBlack
                hoveredColor: JamiTheme.buttonTintedBlackHovered
                pressedColor: JamiTheme.buttonTintedBlackPressed
                outlined: true

                source: "qrc:/images/icons/delete-24px.svg"

                text: qsTr("Uninstall")

                onClicked: uninstallPluginSlot()
            }
        }

        ListView {
            id: pluginPreferenceView

            Layout.fillWidth: true
            Layout.minimumHeight: 0
            Layout.preferredHeight: childrenRect.height + 30

            model: PluginAdapter.getPluginPreferencesModel(pluginId)
            interactive: false

            delegate: PreferenceItemDelegate {
                id: preferenceItemDelegate

                width: pluginPreferenceView.width
                height: 50

                preferenceName: PreferenceName
                preferenceSummary: PreferenceSummary
                preferenceType: PreferenceType
                preferenceCurrentValue: PreferenceCurrentValue
                pluginId: PluginId
                currentPath: CurrentPath
                preferenceKey: PreferenceKey
                fileFilters: FileFilters
                isImage: IsImage
                pluginListPreferenceModel: PluginListPreferenceModel {
                    id: pluginListPreferenceModel
                    preferenceKey : PreferenceKey
                    pluginId: PluginId
                }

                onBtnPreferenceClicked: {
                    setPreference(pluginId, preferenceKey, preferenceNewValue)
                    pluginPreferenceView.model = PluginAdapter.getPluginPreferencesModel(pluginId)
                }

                background: Rectangle {
                    anchors.fill: parent
                    color: "white"
                }
            }
        }
    }
}
