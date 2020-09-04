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

ListView {
    id: root

    signal needToAccessMessageWebView(string currentUserDisplayName, string currentUserAlias, string currentUID, bool callStackViewShouldShow, bool isAudioOnly, int callState)
    signal needToSelectItems(string conversationUid)
    signal needToDeselectItems
    signal needToBackToWelcomePage
    signal needToGrabFocus

    signal needToShowChatView(string accountId, string convUid)
    signal currentIndexIsChanged
    signal forceUpdatePotentialInvalidItem

    // Refresh all items within the model.
    function updateListView() {
        root.model.dataChanged(
                    root.model.index(0, 0),
                    root.model.index(
                    root.model.rowCount() - 1, 0))
        root.forceUpdatePotentialInvalidItem()
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
        function onModelSorted(uri) {
            root.currentIndex = -1
            updateListView()
            for (var i = 0; i < count; i++) {
                if (root.model.data(
                    root.model.index(i, 0), 261) === uri) {
                    root.currentIndex = i
                    break
                }
            }
        }

        function onUpdateListViewRequested() {
            updateListView()
        }

        function onNavigateToWelcomePageRequested() {
            root.needToBackToWelcomePage()
        }
    }

    Connections {
        target: CallAdapter

        function onUpdateConversationSmartList() {
            updateListView()
        }
    }

    onCurrentIndexChanged: {
        root.currentIndexIsChanged()
    }

    clip: true

    delegate: ConversationSmartListViewItemDelegate {
        id: smartListItemDelegate
    }

    ScrollIndicator.vertical: ScrollIndicator {}

    Shortcut {
        sequence: "Ctrl+Shift+X"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            CallAdapter.placeCall()
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+C"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            CallAdapter.placeAudioOnlyCall()
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+L"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            UtilsAdapter.clearConversationHistory(UtilsAdapter.getCurrAccId(),
                                                  UtilsAdapter.getCurrConvId())
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+B"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            UtilsAdapter.removeConversation(UtilsAdapter.getCurrAccId(),
                                            UtilsAdapter.getCurrConvId(),
                                            true)
            root.needToBackToWelcomePage()
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+Delete"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: {
            UtilsAdapter.removeConversation(UtilsAdapter.getCurrAccId(),
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
