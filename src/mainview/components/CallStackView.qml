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

import QtQuick
import QtQuick.Controls

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: callStackViewWindow

    property bool isAudioOnly: false
    property var sipKeys: [
        "1", "2", "3", "A",
        "4", "5", "6", "B",
        "7", "8", "9", "C",
        "*", "0", "#", "D"
    ]

    enum StackNumber {
        InitialPageStack,
        OngoingPageStack
    }

    anchors.fill: parent

    Shortcut {
        sequence: "Ctrl+D"
        context: Qt.ApplicationShortcut
        onActivated: CallAdapter.hangUpThisCall()
        onActivatedAmbiguously: CallAdapter.hangUpThisCall()
    }

    Keys.onPressed: {
        if (LRCInstance.currentAccountType !== Profile.Type.SIP)
            return
        var key = event.text.toUpperCase()
        if(sipKeys.find(function (item) {
            return item === key
        })) {
            CallAdapter.sipInputPanelPlayDTMF(key)
        }
    }

    // When selected conversation is changed,
    // these values will also be changed.
    property string responsibleConvUid: ""
    property string responsibleAccountId: ""

    // TODO: this should all be done by listening to
    // parent visibility change or parent `Component.onDestruction`
    function needToCloseInCallConversationAndPotentialWindow() {
        // Close potential window, context menu releated windows.
        ongoingCallPage.closeInCallConversation()
        ongoingCallPage.closeContextMenuAndRelatedWindows()
    }

    function setLinkedWebview(webViewId) {
        ongoingCallPage.setLinkedWebview(webViewId)
    }

    function getItemFromStack(itemNumber) {
        return callStackMainView.find(function (item) {
            return item.stackNumber === itemNumber
        })
    }

    function showInitialCallPage(callState, isAudioOnly) {
        var itemToFind = getItemFromStack(CallStackView.InitialPageStack)
        if (!itemToFind) {
            callStackMainView.push(initialCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        initialCallPage.callStatus = callState
        initialCallPage.isAudioOnly = isAudioOnly
        if (initialCallPage.callStatus === Call.Status.INCOMING_RINGING)
            initialCallPage.isIncoming = true
        else
            initialCallPage.isIncoming = false
    }

    function showOngoingCallPage() {
        var itemToFind = getItemFromStack(CallStackView.OngoingPageStack)
        if (!itemToFind) {
            callStackMainView.push(ongoingCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        ongoingCallPage.accountPeerPair = [responsibleAccountId, responsibleConvUid]
    }

    function toggleFullScreen() {
        var callPage = callStackMainView.currentItem
        if (!callPage)
            return

        // manual toggle here because of our fake fullscreen mode (F11)
        // TODO: handle and save window states, not just a boolean isFullScreen
        if (!appWindow.isFullScreen && !JamiQmlUtils.callIsFullscreen)
            appWindow.isFullScreen = true
        else if (JamiQmlUtils.callIsFullscreen)
            appWindow.isFullScreen = false

        JamiQmlUtils.callIsFullscreen = !JamiQmlUtils.callIsFullscreen
        callPage.parent = JamiQmlUtils.callIsFullscreen ?
                    appContainer :
                    callStackMainView
        if (!root.isAudioOnly) {
            ongoingCallPage.handleParticipantsInfo(CallAdapter.getConferencesInfos())
        }
    }

    Connections {
        target: JamiQmlUtils

        function onFullScreenCallEnded() {
            if (appWindow.isFullScreen) {
                toggleFullScreen()
            }
        }
    }

    Connections {
        target: CallAdapter

        function onCallInfosChanged(audioOnly, accountId, convUid) {
            if (callStackMainView.currentItem.stackNumber === CallStackView.OngoingPageStack
                    && responsibleConvUid === convUid && responsibleAccountId === accountId) {
                ongoingCallPage.isAudioOnly = audioOnly
            }
        }

        function onCallStatusChanged(status, accountId, convUid) {
            if (callStackMainView.currentItem.stackNumber === CallStackView.InitialPageStack
                    && responsibleConvUid === convUid && responsibleAccountId === accountId) {
                initialCallPage.callStatus = status
            }
        }

        function onUpdateParticipantsInfos(infos, accountId, callId) {
            if (callStackMainView.currentItem.stackNumber === CallStackView.OngoingPageStack && !root.isAudioOnly) {
                var responsibleCallId = UtilsAdapter.getCallId(responsibleAccountId, responsibleConvUid)
                if (responsibleCallId === callId) {
                    ongoingCallPage.handleParticipantsInfo(infos)
                }
            }
        }
    }

    OngoingCallPage {
        id: ongoingCallPage

        anchors.fill: parent
        property int stackNumber: CallStackView.OngoingPageStack

        isAudioOnly: callStackViewWindow.isAudioOnly

        visible: callStackMainView.currentItem.stackNumber === stackNumber
    }

    InitialCallPage {
        id: initialCallPage

        anchors.fill: parent
        property int stackNumber: CallStackView.InitialPageStack

        isAudioOnly: callStackViewWindow.isAudioOnly

        onCallAccepted: {
            CallAdapter.acceptACall(responsibleAccountId, responsibleConvUid)
            mainViewSidePanel.selectTab(SidePanelTabBar.Conversations)
        }

        onCallCanceled: {
            CallAdapter.hangUpACall(responsibleAccountId, responsibleConvUid)
        }

        visible: callStackMainView.currentItem.stackNumber === stackNumber
    }

    StackView {
        id: callStackMainView

        anchors.fill: parent

        initialItem: initialCallPage
    }
}
