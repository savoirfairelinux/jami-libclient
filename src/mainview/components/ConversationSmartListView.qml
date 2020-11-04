/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

ListView {
    id: root

    signal needToDeselectItems
    signal forceUpdatePotentialInvalidItem

    // Refresh all items within the model.
    function updateListView() {
        if (!root.model)
            return
        root.model.dataChanged(
                    root.model.index(0, 0),
                    root.model.index(
                    root.model.rowCount() - 1, 0))
        root.forceUpdatePotentialInvalidItem()
    }

    function repositionIndex(uid = "") {
        if (uid === "")
            uid = mainView.currentConvUID
        root.currentIndex = -1
        updateListView()
        for (var i = 0; i < count; i++) {
            if (root.model.data(
                root.model.index(i, 0), SmartListModel.UID) === uid) {
                root.currentIndex = i
                break
            }
        }
    }

    ConversationSmartListContextMenu {
        id: smartListContextMenu
    }

    Connections {
        target: ConversationsAdapter

        function onModelChanged(model) {
            root.model = model
        }

        // When the model has been sorted, we need to adjust the focus (currentIndex)
        // to the previously focused conversation item.
        function onModelSorted(uid) {
            repositionIndex(uid)
        }

        function onUpdateListViewRequested() {
            updateListView()
        }

        function onIndexRepositionRequested() {
            repositionIndex()
        }
    }

    Connections {
        target: LRCInstance
        function onUpdateSmartList() { updateListView() }
    }

    clip: true

    delegate: ConversationSmartListViewItemDelegate {
        id: smartListItemDelegate

        onUpdateContactAvatarUidRequested: root.model.updateContactAvatarUid(uid)
    }

    ScrollIndicator.vertical: ScrollIndicator {}

    Shortcut {
        sequence: "Ctrl+Shift+X"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            CallAdapter.placeCall()
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+C"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            CallAdapter.placeAudioOnlyCall()
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+L"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            UtilsAdapter.clearConversationHistory(AccountAdapter.currentAccountId,
                                                  UtilsAdapter.getCurrConvId())
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+B"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            MessagesAdapter.blockConversation(UtilsAdapter.getCurrConvId())
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+Delete"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            UtilsAdapter.removeConversation(AccountAdapter.currentAccountId,
                                            UtilsAdapter.getCurrConvId(),
                                            false)
        }
    }

    Shortcut {
        sequence: "Ctrl+Down"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            if (currentIndex + 1 >= count)
                return
            root.currentIndex += 1
        }
    }

    Shortcut {
        sequence: "Ctrl+Up"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            if (currentIndex <= 0)
                return
            root.currentIndex -= 1
        }
    }
}
