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

import "../js/incomingcallpagecreation.js" as IncomingCallPageCreation
import "../js/callfullscreenwindowcontainercreation.js" as CallFullScreenWindowContainerCreation

Rectangle {
    id: callStackViewWindow

    enum StackNumber {
        IncomingPageStack,
        OutgoingPageStack,
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

    function needToCloseInCallConversationAndPotentialWindow() {
        // Close potential window, context menu releated windows.
        if (!callStackMainView.currentItem)
            return
        if (callStackMainView.currentItem.stackNumber === CallStackView.AudioPageStack) {
            audioCallPage.closeInCallConversation()
            CallFullScreenWindowContainerCreation.closeVideoCallFullScreenWindowContainer()
            audioCallPage.closeContextMenuAndRelatedWindows()
        } else if (callStackMainView.currentItem.stackNumber === CallStackView.VideoPageStack) {
            videoCallPage.closeInCallConversation()
            CallFullScreenWindowContainerCreation.closeVideoCallFullScreenWindowContainer()
            videoCallPage.closeContextMenuAndRelatedWindows()
        }
    }

    function setLinkedWebview(webViewId) {
        if (callStackMainView.currentItem.stackNumber === CallStackView.AudioPageStack) {
            audioCallPage.setLinkedWebview(webViewId)
        } else if (callStackMainView.currentItem.stackNumber === CallStackView.VideoPageStack) {
            videoCallPage.setLinkedWebview(webViewId)
        }
    }

    function getItemFromStack(itemNumber) {
        return callStackMainView.find(function (item) {
            return item.stackNumber === itemNumber
        })
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

    function showOutgoingCallPage() {
        var itemToFind = getItemFromStack(CallStackView.OutgoingPageStack)
        if (!itemToFind) {
            callStackMainView.push(outgoingCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        outgoingCallPage.updateUI(responsibleAccountId, responsibleConvUid)
    }

    function showIncomingCallPage(accountId, convUid) {
        var itemToFind = getItemFromStack(CallStackView.IncomingPageStack)
        if (!itemToFind) {
            callStackMainView.push(incomingCallPage, StackView.Immediate)
        } else {
            callStackMainView.pop(itemToFind, StackView.Immediate)
        }
        incomingCallPage.updateUI(responsibleAccountId, responsibleConvUid)
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
        JamiQmlUtils.callIsFullscreen = !JamiQmlUtils.callIsFullscreen
        var callPage = callStackMainView.currentItem
        if (!callPage)
            return
        CallFullScreenWindowContainerCreation.createvideoCallFullScreenWindowContainerObject()

        if (!CallFullScreenWindowContainerCreation.checkIfVisible()) {
            CallFullScreenWindowContainerCreation.setAsContainerChild(callPage)
            CallFullScreenWindowContainerCreation.showVideoCallFullScreenWindowContainer()
        } else {
            callPage.parent = callStackMainView
            CallFullScreenWindowContainerCreation.closeVideoCallFullScreenWindowContainer()
        }

        if (callPage.stackNumber === CallStackView.VideoPageStack) {
            videoCallPage.handleParticipantsInfo(CallAdapter.getConferencesInfos())
        }
    }

    Connections {
        target: CallAdapter

        function onCallStatusChanged(status, accountId, convUid) {
            if (callStackMainView.currentItem.stackNumber === CallStackView.OutgoingPageStack
                    && responsibleConvUid === convUid && responsibleAccountId === accountId) {
                outgoingCallPage.callStatus = status
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

    OutgoingCallPage {
        id: outgoingCallPage

        property int stackNumber: CallStackView.OutgoingPageStack

        visible: callStackMainView.currentItem.stackNumber === stackNumber

        onCallCancelButtonIsClicked: {
            CallAdapter.hangUpACall(responsibleAccountId, responsibleConvUid)
        }
    }

    VideoCallPage {
        id: videoCallPage

        property int stackNumber: CallStackView.VideoPageStack

        visible: callStackMainView.currentItem.stackNumber === stackNumber
    }

    IncomingCallPage {
        id: incomingCallPage

        property int stackNumber: CallStackView.IncomingPageStack

        onCallAcceptButtonIsClicked: {
            CallAdapter.acceptACall(responsibleAccountId, responsibleConvUid)
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
            mainViewSidePanel.selectTab(SidePanelTabBar.Conversations)
        }

        onCallCancelButtonIsClicked: {
            CallAdapter.refuseACall(responsibleAccountId, responsibleConvUid)
        }

        visible: callStackMainView.currentItem.stackNumber === stackNumber
    }

    StackView {
        id: callStackMainView

        anchors.fill: parent

        initialItem: outgoingCallPage
    }
}
