
/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
    id: conversationSmartListView

    signal needToAccessMessageWebView(string currentUserDisplayName, string currentUserAlias, string currentUID, bool callStackViewShouldShow, bool isAudioOnly, string callStateStr)
    signal needToSelectItems(string conversationUid)
    signal needToDeselectItems
    signal needToBackToWelcomePage
    signal needToGrabFocus

    signal needToShowChatView(string accountId, string convUid)
    signal currentIndexIsChanged
    signal forceUpdatePotentialInvalidItem


    /*
     * When model is sorted, we need to reset to focus (currentIndex)
     * to the real conversation that we focused.
     */
    function modelSorted(contactURIToCompare) {
        var conversationSmartListViewModel = conversationSmartListView.model
        conversationSmartListView.currentIndex = -1
        updateConversationSmartListView()
        for (var i = 0; i < count; i++) {
            if (conversationSmartListViewModel.data(
                        conversationSmartListViewModel.index(i, 0),
                        261) === contactURIToCompare) {
                conversationSmartListView.currentIndex = i
                break
            }
        }
    }


    /*
     * Refresh all item within model.
     */
    function updateConversationSmartListView() {
        var conversationSmartListViewModel = conversationSmartListView.model
        conversationSmartListViewModel.dataChanged(
                    conversationSmartListViewModel.index(0, 0),
                    conversationSmartListViewModel.index(
                        conversationSmartListViewModel.rowCount() - 1, 0))
        conversationSmartListView.forceUpdatePotentialInvalidItem()
    }

    function setModel(model) {
        conversationSmartListView.model = model
    }

    function backToWelcomePage() {
        conversationSmartListView.needToBackToWelcomePage()
    }


    /*
     * Update smartlist to accountId.
     */
    function updateSmartList(accountId) {
        conversationSmartListView.model.setAccount(accountId)
    }

    Connections {
        target: CallAdapter

        function onUpdateConversationSmartList() {
            updateConversationSmartListView()
        }
    }

    onCurrentIndexChanged: {
        conversationSmartListView.currentIndexIsChanged()
    }

    clip: true

    delegate: ConversationSmartListViewItemDelegate {
        id: smartListItemDelegate
    }

    ScrollIndicator.vertical: ScrollIndicator {}
}
