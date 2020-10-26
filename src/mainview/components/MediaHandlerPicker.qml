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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"

Popup {
    id: root
    function toggleMediaHandlerSlot(mediaHandlerId, isLoaded) {
        var callId = UtilsAdapter.getCallId(callStackViewWindow.responsibleAccountId,
                                            callStackViewWindow.responsibleConvUid)
        PluginModel.toggleCallMediaHandler(mediaHandlerId, callId, !isLoaded)
        mediahandlerPickerListView.model = PluginAdapter.getMediaHandlerSelectableModel(callId)
    }

    width: 350
    height: contentItem.height

    modal: true

    contentItem: StackLayout {
        id: stack
        currentIndex: 0
        height: childrenRect.height

        Rectangle {
            id: mediahandlerPickerPopupRect
            width: root.width
            height: childrenRect.height + 50
            color: "white"
            radius: 10

            PushButton {
                id: closeButton

                anchors.top: mediahandlerPickerPopupRect.top
                anchors.topMargin: 5
                anchors.right: mediahandlerPickerPopupRect.right
                anchors.rightMargin: 5

                source: "qrc:/images/icons/round-close-24px.svg"

                onClicked: {
                    root.close()
                }
            }

            ColumnLayout {
                id: mediahandlerPickerPopupRectColumnLayout

                anchors.top: mediahandlerPickerPopupRect.top
                anchors.topMargin: 15
                height: 230

                Text {
                    id: mediahandlerPickerTitle

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: mediahandlerPickerPopupRect.width
                    Layout.preferredHeight: 30

                    font.pointSize: JamiTheme.textFontSize
                    font.bold: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    text: qsTr("Choose plugin")
                }

                ListView {
                    id: mediahandlerPickerListView

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: mediahandlerPickerPopupRect.width
                    Layout.preferredHeight: 200

                    model: {
                        var callId = UtilsAdapter.getCallId(callStackViewWindow.responsibleAccountId,
                                                            callStackViewWindow.responsibleConvUid)
                        return PluginAdapter.getMediaHandlerSelectableModel(callId)
                        }

                    clip: true

                    delegate: MediaHandlerItemDelegate {
                        id: mediaHandlerItemDelegate
                        visible: PluginModel.getPluginsEnabled()
                        width: mediahandlerPickerListView.width
                        height: 50

                        mediaHandlerName : MediaHandlerName
                        mediaHandlerId: MediaHandlerId
                        mediaHandlerIcon: MediaHandlerIcon
                        isLoaded: IsLoaded
                        pluginId: PluginId

                        onBtnLoadMediaHandlerToggled: {
                            toggleMediaHandlerSlot(mediaHandlerId, isLoaded)
                        }

                        onOpenPreferences: {
                            mediahandlerPreferencePickerListView.pluginId = pluginId
                            mediahandlerPreferencePickerListView.mediaHandlerName = mediaHandlerName
                            mediahandlerPreferencePickerListView.model = PluginAdapter.getPluginPreferencesModel(pluginId, mediaHandlerName)
                            stack.currentIndex = 1
                        }
                    }

                    ScrollIndicator.vertical: ScrollIndicator {}
                }
            }
        }

        Rectangle {
            id: mediahandlerPreferencePopupRect2
            width: root.width
            height: childrenRect.height + 50
            color: "white"
            radius: 10

            PushButton {
                id: backButton
                anchors.top: mediahandlerPreferencePopupRect2.top
                anchors.topMargin: 5
                anchors.left: mediahandlerPreferencePopupRect2.left
                anchors.leftMargin: 5

                source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                toolTipText: qsTr("Go back to plugins list")
                hoverEnabled: true
                onClicked: {
                    stack.currentIndex = 0
                }
            }

            PushButton {
                id: closeButton2

                anchors.top: mediahandlerPreferencePopupRect2.top
                anchors.topMargin: 5
                anchors.right: mediahandlerPreferencePopupRect2.right
                anchors.rightMargin: 5

                source: "qrc:/images/icons/round-close-24px.svg"

                onClicked: {
                    stack.currentIndex = 0
                    root.close()
                }
            }

            ColumnLayout {

                anchors.top: mediahandlerPreferencePopupRect2.top
                anchors.topMargin: 15
                height: 230

                Text {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: mediahandlerPreferencePopupRect2.width
                    Layout.preferredHeight: 30

                    font.pointSize: JamiTheme.textFontSize
                    font.bold: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    text: qsTr("Preferences")
                }

                ListView {
                    id: mediahandlerPreferencePickerListView
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: mediahandlerPickerPopupRect.width
                    Layout.preferredHeight: 200

                    property string pluginId: ""
                    property string mediaHandlerName: ""

                    model: PluginAdapter.getPluginPreferencesModel(pluginId, mediaHandlerName)

                    clip: true

                    delegate: PreferenceItemDelegate {
                        id: mediaHandlerPreferenceDelegate
                        width: mediahandlerPreferencePickerListView.width
                        height: childrenRect.height

                        preferenceName: PreferenceName
                        preferenceSummary: PreferenceSummary
                        preferenceType: PreferenceType
                        preferenceCurrentValue: PreferenceCurrentValue
                        pluginId: PluginId
                        currentPath: CurrentPath
                        preferenceKey : PreferenceKey
                        fileFilters: FileFilters
                        isImage: IsImage
                        pluginListPreferenceModel: PluginListPreferenceModel{
                            id: pluginListPreferenceModel
                            preferenceKey : PreferenceKey
                            pluginId: PluginId
                        }

                        onClicked:  mediahandlerPreferencePickerListView.currentIndex = index

                        onBtnPreferenceClicked: {
                            PluginModel.setPluginPreference(pluginId, preferenceKey, preferenceNewValue)
                            mediahandlerPreferencePickerListView.model = PluginAdapter.getPluginPreferencesModel(pluginId, mediahandlerPreferencePickerListView.mediaHandlerName)
                        }
                    }

                    ScrollIndicator.vertical: ScrollIndicator {}
                }
            }
        }
    }

    onAboutToHide: stack.currentIndex = 0

    onAboutToShow: {
        // Reset the model on each show.
        var callId = UtilsAdapter.getCallId(callStackViewWindow.responsibleAccountId,
                                            callStackViewWindow.responsibleConvUid)
        mediahandlerPickerListView.model = PluginAdapter.getMediaHandlerSelectableModel(callId)
    }

    background: Rectangle {
        color: "transparent"
    }
}
