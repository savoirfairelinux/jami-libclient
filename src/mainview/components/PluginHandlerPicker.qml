/*
 * Copyright (C) 2021 by Savoir-faire Linux
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
import net.jami.Constants 1.0

import "../../commoncomponents"

Popup {
    id: root

    property bool isCall
    property string pluginId: ""
    property string handlerName: ""
    signal updateProperties

    width: JamiTheme.preferredDialogWidth
    height: JamiTheme.pluginHandlersPopupViewHeight + JamiTheme.pluginHandlersPopupViewDelegateHeight

    modal: true

    contentItem: StackView {
        id: stack
        initialItem: pluginhandlerPreferenceStack
        anchors.fill: parent
    }

    Component {
        id: pluginhandlerPreferenceStack

        Rectangle {
            color: JamiTheme.backgroundColor
            radius: 10
            anchors.fill: parent

            Connections {
                target: root

                function onAboutToShow(visible) {
                    if (isCall) {
                        // Reset the model on each show.
                        var callId = UtilsAdapter.getCallId(callStackViewWindow.responsibleAccountId,
                                                            callStackViewWindow.responsibleConvUid)
                        pluginhandlerPickerListView.model = PluginAdapter.getMediaHandlerSelectableModel(callId)
                    } else {
                        // Reset the model on each show.
                        var accountId = AccountAdapter.currentAccountId
                        var peerId = UtilsAdapter.getPeerUri(accountId, UtilsAdapter.getCurrConvId())
                        pluginhandlerPickerListView.model = PluginAdapter.getChatHandlerSelectableModel(accountId, peerId)
                    }
                }
            }

            function toggleHandlerSlot(handlerId, isLoaded) {
                if (isCall) {
                    var callId = UtilsAdapter.getCallId(callStackViewWindow.responsibleAccountId,
                                                    callStackViewWindow.responsibleConvUid)
                    PluginModel.toggleCallMediaHandler(handlerId, callId, !isLoaded)
                    pluginhandlerPickerListView.model = PluginAdapter.getMediaHandlerSelectableModel(callId)
                } else {
                    var accountId = AccountAdapter.currentAccountId
                    var peerId = UtilsAdapter.getPeerUri(accountId, UtilsAdapter.getCurrConvId())
                    PluginModel.toggleChatHandler(handlerId, accountId, peerId, !isLoaded)
                    pluginhandlerPickerListView.model = PluginAdapter.getChatHandlerSelectableModel(accountId, peerId)
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.bottomMargin: 5

                RowLayout {
                    height: JamiTheme.preferredFieldHeight

                    Text {
                        Layout.topMargin: 10
                        Layout.leftMargin: 5 + closeButton.width
                        Layout.alignment: Qt.AlignCenter
                        Layout.fillWidth: true

                        font.pointSize: JamiTheme.textFontSize
                        font.bold: true

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: JamiTheme.textColor

                        text: qsTr("Choose plugin")
                    }

                    PushButton {
                        id: closeButton
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        Layout.topMargin: 5

                        source: "qrc:/images/icons/round-close-24px.svg"
                        imageColor: JamiTheme.textColor

                        onClicked: {
                            root.close()
                        }
                    }
                }

                ListViewJami {
                    id: pluginhandlerPickerListView

                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: 0

                    model: {
                        if (isCall) {
                            var callId = UtilsAdapter.getCallId(callStackViewWindow.responsibleAccountId,
                                                                callStackViewWindow.responsibleConvUid)
                            return PluginAdapter.getMediaHandlerSelectableModel(callId)
                        } else {
                            var accountId = AccountAdapter.currentAccountId
                            var peerId = UtilsAdapter.getPeerUri(accountId, UtilsAdapter.getCurrConvId())
                            return PluginAdapter.getChatHandlerSelectableModel(accountId, peerId)
                        }
                    }

                    clip: true

                    delegate: PluginHandlerItemDelegate {
                        id: pluginHandlerItemDelegate
                        visible: PluginModel.getPluginsEnabled()
                        width: pluginhandlerPickerListView.width
                        height: JamiTheme.pluginHandlersPopupViewDelegateHeight

                        handlerName : HandlerName
                        handlerId: HandlerId
                        handlerIcon: HandlerIcon
                        isLoaded: IsLoaded
                        pluginId: PluginId

                        onBtnLoadHandlerToggled: {
                            toggleHandlerSlot(HandlerId, isLoaded)
                        }

                        onOpenPreferences: {
                            root.pluginId = pluginId
                            root.handlerName = handlerName
                            stack.push(pluginhandlerPreferenceStack2, StackView.Immediate)
                            updateProperties()
                        }
                    }

                    ScrollIndicator.vertical: ScrollIndicator {}
                }
            }
        }
    }

    Component {
        id: pluginhandlerPreferenceStack2

        Rectangle {
            color: JamiTheme.backgroundColor
            radius: 10
            anchors.fill: parent

            Connections {
                target: root

                function onUpdateProperties() {
                    pluginhandlerPreferencePickerListView.pluginId = root.pluginId
                    pluginhandlerPreferencePickerListView.handlerName = root.handlerName
                    pluginhandlerPreferencePickerListView.model = PluginAdapter.getHandlerPreferencesModel(root.pluginId, root.handlerName)
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.bottomMargin: 5

                RowLayout {
                    height: JamiTheme.preferredFieldHeight

                    PushButton {
                        id: backButton
                        Layout.leftMargin: 5
                        Layout.topMargin: 5

                        imageColor: JamiTheme.textColor
                        source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                        toolTipText: qsTr("Go back to plugins list")
                        hoverEnabled: true
                        onClicked: {
                            stack.pop(null, StackView.Immediate)
                        }
                    }

                    Text {
                        Layout.topMargin: 10
                        Layout.alignment: Qt.AlignCenter
                        Layout.fillWidth: true

                        font.pointSize: JamiTheme.textFontSize
                        font.bold: true

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        color: JamiTheme.textColor
                        text: qsTr("Preferences")
                    }

                    PushButton {
                        id: closeButton2
                        Layout.rightMargin: 5
                        Layout.topMargin: 5

                        source: "qrc:/images/icons/round-close-24px.svg"
                        imageColor: JamiTheme.textColor

                        onClicked: {
                            root.close()
                        }
                    }
                }

                ListViewJami {
                    id: pluginhandlerPreferencePickerListView
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    property string pluginId: ""
                    property string handlerName: ""

                    model: PluginAdapter.getHandlerPreferencesModel(pluginId, handlerName)
                    border.width: 0

                    clip: true

                    delegate: PreferenceItemDelegate {
                        id: pluginHandlerPreferenceDelegate
                        width: pluginhandlerPreferencePickerListView.width
                        height: JamiTheme.pluginHandlersPopupViewDelegateHeight

                        preferenceName: PreferenceName
                        preferenceSummary: PreferenceSummary
                        preferenceType: PreferenceType
                        preferenceCurrentValue: PreferenceCurrentValue
                        pluginId: PluginId
                        currentPath: CurrentPath
                        preferenceKey : PreferenceKey
                        fileFilters: FileFilters
                        isImage: IsImage
                        enabled: Enabled
                        pluginListPreferenceModel: PluginListPreferenceModel {
                            id: handlerPickerPreferenceModel

                            lrcInstance: LRCInstance
                            preferenceKey : PreferenceKey
                            pluginId: PluginId
                        }

                        onClicked:  pluginhandlerPreferencePickerListView.currentIndex = index

                        onBtnPreferenceClicked: {
                            PluginModel.setPluginPreference(pluginId, preferenceKey, preferenceNewValue)
                            PluginAdapter.preferenceChanged(pluginId)
                            pluginhandlerPreferencePickerListView.model = PluginAdapter.getHandlerPreferencesModel(pluginId, pluginhandlerPreferencePickerListView.handlerName)
                        }
                    }
                }
            }
        }
    }

    onAboutToHide: stack.pop(null, StackView.Immediate)

    background: Rectangle {
        color: "transparent"
    }
}
