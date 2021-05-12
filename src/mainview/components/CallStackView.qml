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
import QtQuick.Controls.Universal 2.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

import "../js/incomingcallpagecreation.js" as IncomingCallPageCreation

Rectangle {
    id: callStackViewWindow

    enum StackNumber {
        InitialPageStack,
        AudioPageStack,
        VideoPageStack
    }

    anchors.fill: parent

    Shortcut {
        sequence: "Ctrl+D"
        context: Qt.ApplicationShortcut
        onActivated: CallAdapter.hangUpThisCall()
        onActivatedAmbiguously: CallAdapter.hangUpThisCall()
    }

    // When selected conversation is changed,
    // these values will also be changed.
    property string responsibleConvUid: ""
    property string responsibleAccountId: ""

    // TODO: this should all be done by listening to
    // parent visibility change or parent `Component.onDestruction`
    function needToCloseInCallConversationAndPotentialWindow() {
        // Close potential window, context menu releated windows.
        if (audioCallPage) {
            audioCallPage.closeInCallConversation()
            audioCallPage.closeContextMenuAndRelatedWindows()
        }
        if (videoCallPage) {
            videoCallPage.closeInCallConversation()
            videoCallPage.closeContextMenuAndRelatedWindows()
        }
    }

    function setLinkedWebview(webViewId) {
        if (audioCallPage)
            audioCallPage.setLinkedWebview(webViewId)
        if (videoCallPage)
            videoCallPage.setLinkedWebview(webViewId)
    }

    function getItemFromStack(itemNumber) {
        return callStackMainView.find(function (item) {
            return item.stackNumber === itemNumber
        })
    }

    function showInitialCallPage(callState) {
        var itemToFind = getItemFromStack(CallStackView.InitialPageStack)
        if (!itemToFind) {
            callStackMainView.push(initialCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        initialCallPage.accountConvPair = [responsibleAccountId, responsibleConvUid]
        initialCallPage.callStatus = callState
        if (initialCallPage.callStatus === Call.Status.INCOMING_RINGING)
            initialCallPage.isIncoming = true
        else
            initialCallPage.isIncoming = false
    }

    function showAudioCallPage() {
        var itemToFind = getItemFromStack(CallStackView.AudioPageStack)
        if (!itemToFind) {
            callStackMainView.push(audioCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        audioCallPage.updateUI(responsibleAccountId, responsibleConvUid)
    }

    function showVideoCallPage() {
        var itemToFind = getItemFromStack(CallStackView.VideoPageStack)
        if (!itemToFind) {
            callStackMainView.push(videoCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        videoCallPage.updateUI(responsibleAccountId, responsibleConvUid)
        var callId = UtilsAdapter.getCallId(responsibleAccountId,
                                            responsibleConvUid)
        videoCallPage.setDistantRendererId(callId)
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
        if (callPage.stackNumber === CallStackView.VideoPageStack) {
            videoCallPage.handleParticipantsInfo(CallAdapter.getConferencesInfos())
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

        function onCallStatusChanged(status, accountId, convUid) {
            if (callStackMainView.currentItem.stackNumber === CallStackView.InitialPageStack
                    && responsibleConvUid === convUid && responsibleAccountId === accountId) {
                initialCallPage.callStatus = status
            }
        }

        function onUpdateParticipantsInfos(infos, accountId, callId) {
            if (callStackMainView.currentItem.stackNumber === CallStackView.VideoPageStack) {
                var responsibleCallId = UtilsAdapter.getCallId(responsibleAccountId, responsibleConvUid)
                if (responsibleCallId === callId) {
                    videoCallPage.handleParticipantsInfo(infos)
                }
            }
        }
    }

    AudioCallPage {
        id: audioCallPage

        property int stackNumber: CallStackView.AudioPageStack

        visible: callStackMainView.currentItem.stackNumber === stackNumber
    }

    VideoCallPage {
        id: videoCallPage

        property int stackNumber: CallStackView.VideoPageStack

        visible: callStackMainView.currentItem.stackNumber === stackNumber
    }

    InitialCallPage {
        id: initialCallPage

        property int stackNumber: CallStackView.InitialPageStack

        onCallAccepted: {
            CallAdapter.acceptACall(responsibleAccountId, responsibleConvUid)
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
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
