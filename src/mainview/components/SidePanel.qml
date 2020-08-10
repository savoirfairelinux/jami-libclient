
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

import "../../commoncomponents"

Rectangle {
    id: sidePanelRect
    color: JamiTheme.backgroundColor

    property bool tabBarVisible: true
    property int pendingRequestCount: 0
    property int totalUnreadMessagesCount: 0

    signal conversationSmartListNeedToAccessMessageWebView(string currentUserDisplayName, string currentUserAlias, string currentUID, bool callStackViewShouldShow, bool isAudioOnly, string callStateStr)
    signal accountComboBoxNeedToShowWelcomePage(int index)
    signal conversationSmartListViewNeedToShowWelcomePage
    signal accountSignalsReconnect(string accountId)
    signal needToUpdateConversationForAddedContact
    signal needToAddNewAccount


    /*
     * Hack -> force redraw.
     */
    function forceReselectConversationSmartListCurrentIndex() {
        var index = conversationSmartListView.currentIndex
        conversationSmartListView.currentIndex = -1
        conversationSmartListView.currentIndex = index
    }


    /*
     * For contact request conv to be focused correctly.
     */
    function setCurrentUidSmartListModelIndex() {
        conversationSmartListView.currentIndex
                = conversationSmartListView.model.currentUidSmartListModelIndex(
                    )
    }

    function updatePendingRequestCount() {
        pendingRequestCount = ClientWrapper.utilsAdaptor.getTotalPendingRequest()
    }

    function updateTotalUnreadMessagesCount() {
        totalUnreadMessagesCount = ClientWrapper.utilsAdaptor.getTotalUnreadMessages()
    }

    function clearContactSearchBar() {
        contactSearchBar.clearText()
    }

    function accountChangedUIReset() {
        contactSearchBar.clearText()
        contactSearchBar.setPlaceholderString(
                    JamiTheme.contactSearchBarPlaceHolderConversationText)
        sidePanelTabBar.converstationTabDown = true
        sidePanelTabBar.invitationTabDown = false
    }

    function needToChangeToAccount(accountId, index) {
        if (index !== -1) {
            accountComboBox.currentIndex = index
            ClientWrapper.accountAdaptor.accountChanged(index)
            accountChangedUIReset()
        }
    }

    function refreshAccountComboBox(index = -1) {
        accountComboBox.resetAccountListModel()


        /*
         * To make sure that the ui is refreshed for accountComboBox.
         * Note: when index in -1, it means to maintain the
         *       current account selection.
         */
        var currentIndex = accountComboBox.currentIndex
        if (accountComboBox.currentIndex === index)
            accountComboBox.currentIndex = -1
        accountComboBox.currentIndex = index
        if (index !== -1)
            ClientWrapper.accountAdaptor.accountChanged(index)
        else
            accountComboBox.currentIndex = currentIndex
        accountComboBox.update()
        accountChangedUIReset()
    }

    function deselectConversationSmartList() {
        ConversationsAdapter.deselectConversation()
        conversationSmartListView.currentIndex = -1
    }

    function forceUpdateConversationSmartListView() {
        conversationSmartListView.updateConversationSmartListView()
    }

    function updateSmartList(accountId) {
        conversationSmartListView.currentIndex = -1
        conversationSmartListView.updateSmartList(accountId)
    }

    /*
     * Intended -> since strange behavior will happen without this for stackview.
     */
    anchors.top: parent.top
    anchors.fill: parent

    /*
     * Search bar container to embed search label
     */
    ContactSearchBar {
        id: contactSearchBar
        width: sidePanelRect.width - 26
        height: 35
        anchors.top: sidePanelRect.top
        anchors.topMargin: 10
        anchors.left: sidePanelRect.left
        anchors.leftMargin: 16

        onContactSearchBarTextChanged: {
            ClientWrapper.utilsAdaptor.setConversationFilter(text)
        }
    }

    SidePanelTabBar {
        id: sidePanelTabBar
        anchors.top: contactSearchBar.bottom
        anchors.topMargin: 10
        width: sidePanelRect.width
        height: tabBarVisible ? 64 : 0
    }


    ConversationSmartListView {
        id: conversationSmartListView

        anchors.top: tabBarVisible ? sidePanelTabBar.bottom : contactSearchBar.bottom
        anchors.topMargin: tabBarVisible ? 0 : 10
        width: parent.width
        height: tabBarVisible ? sidePanelRect.height - sidePanelTabBar.height - contactSearchBar.height - 20 :
                                sidePanelRect.height - contactSearchBar.height - 20

        Connections {
            target: ConversationsAdapter

            function onShowChatView(accountId, convUid) {
                conversationSmartListView.needToShowChatView(accountId,
                                                             convUid)
            }

            function onShowConversationTabs(visible) {
                tabBarVisible = visible
                updatePendingRequestCount()
                updateTotalUnreadMessagesCount()
            }
        }

        onNeedToSelectItems: {
            ConversationsAdapter.selectConversation(index)
        }

        onNeedToBackToWelcomePage: {
            sidePanelRect.conversationSmartListViewNeedToShowWelcomePage()
        }

        onNeedToAccessMessageWebView: {
            sidePanelRect.conversationSmartListNeedToAccessMessageWebView(
                        currentUserDisplayName, currentUserAlias,
                        currentUID, callStackViewShouldShow,
                        isAudioOnly, callStateStr)
        }

        onNeedToGrabFocus: {
            contactSearchBar.clearFocus()
        }

        Component.onCompleted: {
            ConversationsAdapter.setQmlObject(this)
            conversationSmartListView.currentIndex = -1
        }
    }
}
