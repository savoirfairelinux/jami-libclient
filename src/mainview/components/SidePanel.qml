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
import net.jami.Adapters 1.0

import "../../commoncomponents"

Rectangle {
    id: sidePanelRect

    color: JamiTheme.backgroundColor

    property bool tabBarVisible: true
    property int pendingRequestCount: 0
    property int totalUnreadMessagesCount: 0

    signal conversationSmartListNeedToAccessMessageWebView(string currentUserDisplayName, string currentUserAlias, string currentUID, bool callStackViewShouldShow, bool isAudioOnly, int callState)
    signal needToUpdateConversationForAddedContact

    // Hack -> force redraw.
    function forceReselectConversationSmartListCurrentIndex() {
        var index = conversationSmartListView.currentIndex
        conversationSmartListView.currentIndex = -1
        conversationSmartListView.currentIndex = index
    }


    // For contact request conv to be focused correctly.
    function setCurrentUidSmartListModelIndex() {
        conversationSmartListView.currentIndex
                = conversationSmartListView.model.currentUidSmartListModelIndex(
                    )
    }

    function updatePendingRequestCount() {
        pendingRequestCount = UtilsAdapter.getTotalPendingRequest()
    }

    function updateTotalUnreadMessagesCount() {
        totalUnreadMessagesCount = UtilsAdapter.getTotalUnreadMessages()
    }

    function clearContactSearchBar() {
        contactSearchBar.clearText()
    }

    function accountChangedUIReset() {
        contactSearchBar.clearText()
        sidePanelTabBar.converstationTabDown = true
        sidePanelTabBar.invitationTabDown = false
    }

    function refreshAccountComboBox(index) {
        accountComboBox.update()
        accountChangedUIReset()
        accountComboBox.resetAccountListModel()
    }

    function deselectConversationSmartList() {
        ConversationsAdapter.deselectConversation()
        conversationSmartListView.currentIndex = -1
    }

    function forceUpdateConversationSmartListView() {
        conversationSmartListView.updateListView()
    }

    // Intended -> since strange behavior will happen without this for stackview.
    anchors.top: parent.top
    anchors.fill: parent

    // Search bar container to embed search label
    ContactSearchBar {
        id: contactSearchBar
        width: sidePanelRect.width - 26
        height: 35
        anchors.top: sidePanelRect.top
        anchors.topMargin: 10
        anchors.left: sidePanelRect.left
        anchors.leftMargin: 16

        onContactSearchBarTextChanged: {
            UtilsAdapter.setConversationFilter(text)
        }
    }

    SidePanelTabBar {
        id: sidePanelTabBar
        anchors.top: contactSearchBar.bottom
        anchors.topMargin: 10
        width: sidePanelRect.width
        height: tabBarVisible ? 64 : 0
    }

    Rectangle {
        id: searchStatusRect

        visible: lblSearchStatus.text !== ""

        anchors.top: tabBarVisible ? sidePanelTabBar.bottom : contactSearchBar.bottom
        anchors.topMargin: tabBarVisible ? 0 : 10
        width: parent.width
        height: 72

        color: "transparent"

        Image {
            id: searchIcon
            anchors.left: searchStatusRect.left
            anchors.leftMargin: 24
            anchors.verticalCenter: searchStatusRect.verticalCenter
            width: 24
            height: 24

            fillMode: Image.PreserveAspectFit
            mipmap: true
            source: "qrc:/images/icons/ic_baseline-search-24px.svg"
        }

        Label {
            id: lblSearchStatus

            anchors.verticalCenter: searchStatusRect.verticalCenter
            anchors.left: searchIcon.right
            anchors.leftMargin: 24
            width: searchStatusRect.width - searchIcon.width - 24*2 - 8
            text: ""
            wrapMode: Text.WordWrap
            font.pointSize: JamiTheme.menuFontSize
        }

        MouseArea {
            id: mouseAreaSearchRect

            anchors.fill: parent
            hoverEnabled: true

            onReleased: {
                searchStatusRect.color = JamiTheme.releaseColor
            }

            onEntered: {
                searchStatusRect.color = JamiTheme.hoverColor
            }

            onExited: {
                searchStatusRect.color = JamiTheme.backgroundColor
            }
        }
    }

    ConversationSmartListView {
        id: conversationSmartListView

        anchors.top: searchStatusRect.visible ? searchStatusRect.bottom : (tabBarVisible ? sidePanelTabBar.bottom : contactSearchBar.bottom)
        anchors.topMargin: (tabBarVisible || searchStatusRect.visible) ? 0 : 10
        width: parent.width
        height: tabBarVisible ? sidePanelRect.height - sidePanelTabBar.height - contactSearchBar.height - 20 :
                                sidePanelRect.height - contactSearchBar.height - 20

        Connections {
            target: ConversationsAdapter

            function onShowChatView(accountId, convUid) {
                conversationSmartListView.needToShowChatView(accountId, convUid)
            }

            function onShowConversationTabs(visible) {
                tabBarVisible = visible
                updatePendingRequestCount()
                updateTotalUnreadMessagesCount()
            }

            function onShowSearchStatus(status) {
                lblSearchStatus.text = status
            }
        }

        onNeedToSelectItems: {
            ConversationsAdapter.selectConversation(conversationUid)
        }

        onNeedToAccessMessageWebView: {
            sidePanelRect.conversationSmartListNeedToAccessMessageWebView(
                        currentUserDisplayName, currentUserAlias,
                        currentUID, callStackViewShouldShow,
                        isAudioOnly, callState)
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
