/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

import QtQuick
import QtQuick.Controls

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

JamiListView {
    id: root

    // the following should be marked required (Qtver >= 5.15)
    // along with `required model`
    property string headerLabel
    property bool headerVisible

    delegate: SmartListItemDelegate {}
    currentIndex: model.currentFilteredRow

    // highlight selection
    // down and hover states are done within the delegate
    highlight: Rectangle {
        width: ListView.view ? ListView.view.width : 0
        color: JamiTheme.selectedColor
    }
    highlightMoveDuration: 60

    headerPositioning: ListView.OverlayHeader
    header: Rectangle {
        z: 2
        color: JamiTheme.backgroundColor
        visible: root.headerVisible
        width: root.width
        height: root.headerVisible ? 20 : 0
        Text {
            anchors {
                left: parent.left
                leftMargin: 16
                verticalCenter: parent.verticalCenter
            }
            text: headerLabel + " (" + root.count + ")"
            font.pointSize: JamiTheme.smartlistItemFontSize
            font.weight: Font.DemiBold
            color: JamiTheme.textColor
        }
    }

    Connections {
        target: model

        // actually select the conversation
        function onValidSelectionChanged() {
            var row = model.currentFilteredRow
            var convId = model.dataForRow(row, ConversationList.UID)
            LRCInstance.selectConversation(convId)
        }
    }

    onCountChanged: positionViewAtBeginning()

    Component.onCompleted: {
        // TODO: remove this
        ConversationsAdapter.setQmlObject(this)
    }

    add: Transition {
        NumberAnimation {
            property: "opacity"; from: 0; to: 1.0
            duration: JamiTheme.smartListTransitionDuration
        }
    }

    displaced: Transition {
        NumberAnimation {
            properties: "x,y"; easing.type: Easing.OutCubic
            duration: JamiTheme.smartListTransitionDuration
        }
        NumberAnimation {
            property: "opacity"; to: 1.0
            duration: JamiTheme.smartListTransitionDuration * (1 - from)
        }
    }

    Behavior on opacity {
        NumberAnimation {
            easing.type: Easing.OutCubic
            duration: 2 * JamiTheme.smartListTransitionDuration
        }
    }

    function openContextMenuAt(x, y, delegate) {
        var mappedCoord = root.mapFromItem(delegate, x, y)
        contextMenu.openMenuAt(mappedCoord.x, mappedCoord.y)
    }

    ConversationSmartListContextMenu {
        id: contextMenu

        function openMenuAt(x, y) {
            contextMenu.x = x
            contextMenu.y = y

            // TODO:
            // - accountId, convId only
            // - userProfile dialog should use a loader/popup

            var row = root.indexAt(x, y + root.contentY)
            var item = {
                "convId": model.dataForRow(row, ConversationList.UID),
                "displayId": model.dataForRow(row, ConversationList.BestId),
                "title": model.dataForRow(row, ConversationList.Title),
                "uri": model.dataForRow(row, ConversationList.URI),
                "isSwarm": model.dataForRow(row, ConversationList.IsSwarm),
                "mode": model.dataForRow(row, ConversationList.Mode),
                "readOnly": model.dataForRow(row, ConversationList.ReadOnly)
            }

            responsibleAccountId = LRCInstance.currentAccountId
            responsibleConvUid = item.convId
            isSwarm = item.isSwarm
            mode = item.mode
            contactType = LRCInstance.currentAccountType
            readOnly = item.readOnly

            if (model.dataForRow(row, ConversationList.IsCoreDialog)) {
                userProfile.aliasText = item.title
                userProfile.registeredNameText = item.displayId
                userProfile.idText = item.uri
                userProfile.convId = item.convId
                userProfile.isSwarm = item.isSwarm
            }

            openMenu()
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+X"
        context: Qt.ApplicationShortcut
        enabled: CurrentAccount.videoEnabled_Video && root.visible
        onActivated: {
            if (CurrentAccount.videoEnabled_Video)
                CallAdapter.placeCall()
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+C"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: CallAdapter.placeAudioOnlyCall()
    }

    Shortcut {
        sequence: "Ctrl+Shift+L"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: MessagesAdapter.clearConversationHistory(
                         LRCInstance.currentAccountId,
                         LRCInstance.selectedConvUid)
    }

    Shortcut {
        sequence: "Ctrl+Shift+B"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            MessagesAdapter.blockConversation(
                        LRCInstance.selectedConvUid)
        }
    }

    Shortcut {
        sequence: "Ctrl+Down"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            if (currentIndex + 1 >= count)
                return
            model.select(currentIndex + 1)
        }
    }

    Shortcut {
        sequence: "Ctrl+Up"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            if (currentIndex <= 0)
                return
            model.select(currentIndex - 1)
        }
    }
}
