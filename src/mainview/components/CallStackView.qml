
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
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.12
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../js/incomingcallpagecreation.js" as IncomingCallPageCreation
import "../js/callfullscreenwindowcontainercreation.js" as CallFullScreenWindowContainerCreation

Rectangle {
    id: callStackViewWindow

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

    function needToCloseInCallConversationAndPotentialWindow() {
        audioCallPage.closeInCallConversation()
        videoCallPage.closeInCallConversation()

        // Close potential window, context menu releated windows.
        audioCallPage.closeContextMenuAndRelatedWindows()

        CallFullScreenWindowContainerCreation.closeVideoCallFullScreenWindowContainer()
        videoCallPage.closeContextMenuAndRelatedWindows()
    }

    function setLinkedWebview(webViewId) {
        audioCallPage.setLinkedWebview(webViewId)
        videoCallPage.setLinkedWebview(webViewId)
    }

    function updateCorrespondingUI() {
        audioCallPage.updateUI(responsibleAccountId, responsibleConvUid)
        outgoingCallPage.updateUI(responsibleAccountId, responsibleConvUid)
        incomingCallPage.updateUI(responsibleAccountId, responsibleConvUid)
        videoCallPage.updateUI(responsibleAccountId, responsibleConvUid)
    }

    function showAudioCallPage() {
        var itemToFind = callStackMainView.find(function (item) {
            return item.stackNumber === 0
        })

        if (!itemToFind) {
            callStackMainView.push(audioCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        audioCallPage.updateUI(responsibleAccountId, responsibleConvUid)
    }

    function showOutgoingCallPage() {
        var itemToFind = callStackMainView.find(function (item) {
            return item.stackNumber === 1
        })

        if (!itemToFind) {
            callStackMainView.push(outgoingCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
    }

    function showIncomingCallPage(accountId, convUid) {
        var itemToFind = callStackMainView.find(function (item) {
            return item.stackNumber === 3
        })

        if (!itemToFind) {
            callStackMainView.push(incomingCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        responsibleAccountId = accountId
        responsibleConvUid = convUid
        incomingCallPage.updateUI(accountId, convUid)
    }

    function showVideoCallPage() {
        var itemToFind = callStackMainView.find(function (item) {
            return item.stackNumber === 2
        })

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

    function toogleFullScreen(callPage){
        callPage.isFullscreen = !callPage.isFullscreen
        CallFullScreenWindowContainerCreation.createvideoCallFullScreenWindowContainerObject()

        if (!CallFullScreenWindowContainerCreation.checkIfVisible()) {
            CallFullScreenWindowContainerCreation.setAsContainerChild(
                        callPage)
            CallFullScreenWindowContainerCreation.showVideoCallFullScreenWindowContainer()
        } else {
            callPage.parent = callStackMainView
            CallFullScreenWindowContainerCreation.closeVideoCallFullScreenWindowContainer()
        }
    }

    Connections {
        target: CallAdapter

        function onCallStatusChanged(status, accountId, convUid) {
            if (responsibleConvUid === convUid && responsibleAccountId === accountId) {
                outgoingCallPage.callStatus = status
            }
        }

        function onUpdateParticipantsInfos(infos, accountId, callId) {
            var responsibleCallId = UtilsAdapter.getCallId(responsibleAccountId, responsibleConvUid)
            if (responsibleCallId === callId) {
                videoCallPage.handleParticipantsInfo(infos)
            }
        }
    }

    AudioCallPage {
        id: audioCallPage

        property int stackNumber: 0
        property bool isFullscreen: false

        onShowFullScreenReqested: toogleFullScreen(this)
    }

    OutgoingCallPage {
        id: outgoingCallPage

        property int stackNumber: 1

        onCallCancelButtonIsClicked: {
            CallAdapter.hangUpACall(responsibleAccountId, responsibleConvUid)
        }
    }

    VideoCallPage {
        id: videoCallPage

        property int stackNumber: 2
        property bool isFullscreen: false

        onShowFullScreenReqested: {
            toogleFullScreen(this)
            videoCallPage.handleParticipantsInfo(CallAdapter.getConferencesInfos())
        }
    }

    IncomingCallPage {
        id: incomingCallPage

        property int stackNumber: 3

        onCallAcceptButtonIsClicked: {
            CallAdapter.acceptACall(responsibleAccountId, responsibleConvUid)
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
            mainViewWindowSidePanel.selectTab(SidePanelTabBar.Conversations)
        }

        onCallCancelButtonIsClicked: {
            CallAdapter.hangUpACall(responsibleAccountId, responsibleConvUid)
        }
    }

    StackView {
        id: callStackMainView

        anchors.fill: parent

        initialItem: outgoingCallPage
    }
}
