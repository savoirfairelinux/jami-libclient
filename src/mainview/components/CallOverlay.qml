/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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
import QtQml 2.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../js/contactpickercreation.js" as ContactPickerCreation
import "../js/pluginhandlerpickercreation.js" as PluginHandlerPickerCreation

import "../../commoncomponents"

Rectangle {
    id: root

    property string timeText: "00:00"
    property string remoteRecordingLabel: ""
    property bool isVideoMuted: true
    property bool isAudioOnly: false
    property string bestName: ""

    property var participantOverlays: []
    property var participantComponent: Qt.createComponent("ParticipantOverlay.qml")

    signal overlayChatButtonClicked

    onVisibleChanged: if (!visible) callViewContextMenu.close()

    function setRecording(localIsRecording) {
        callViewContextMenu.localIsRecording = localIsRecording
        recordingRect.visible = localIsRecording
                || callViewContextMenu.peerIsRecording
    }

    function updateButtonStatus(isPaused, isAudioOnly, isAudioMuted, isVideoMuted,
                                isRecording, isSIP, isConferenceCall) {
        callViewContextMenu.isSIP = isSIP
        callViewContextMenu.isPaused = isPaused
        callViewContextMenu.isAudioOnly = isAudioOnly
        callViewContextMenu.localIsRecording = isRecording
        recordingRect.visible = isRecording
        callOverlayButtonGroup.setButtonStatus(isPaused, isAudioOnly,
                                               isAudioMuted, isVideoMuted,
                                               isSIP, isConferenceCall)
    }

    function updateMenu() {
        callOverlayButtonGroup.updateMenu()
    }

    function showOnHoldImage(visible) {
        onHoldImage.visible = visible
    }

    function closePotentialContactPicker() {
        ContactPickerCreation.closeContactPicker()
    }

    function closePotentialPluginHandlerPicker() {
        PluginHandlerPickerCreation.closePluginHandlerPicker()
    }

    // returns true if participant is not fully maximized
    function showMaximize(pX, pY, pW, pH) {
        // Hack: -1 offset added to avoid problems with odd sizes
        return (pX - distantRenderer.getXOffset() !== 0
                || pY - distantRenderer.getYOffset() !== 0
                || pW < (distantRenderer.width - distantRenderer.getXOffset() * 2 - 1)
                || pH < (distantRenderer.height - distantRenderer.getYOffset() * 2 - 1))
    }

    function handleParticipantsInfo(infos) {
        if (root.isAudioOnly)
            return;
        // TODO: in the future the conference layout should be entirely managed by the client
        // Hack: truncate and ceil participant's overlay position and size to correct
        // when they are not exacts
        callOverlay.updateMenu()
        var showMax = false
        var showMin = false

        var deletedUris = []
        var currentUris = []
        for (var p in participantOverlays) {
            if (participantOverlays[p]) {
                var participant = infos.find(e => e.uri === participantOverlays[p].uri);
                if (participant) {
                    // Update participant's information
                    var newX = Math.trunc(distantRenderer.getXOffset()
                            + participant.x * distantRenderer.getScaledWidth())
                    var newY = Math.trunc(distantRenderer.getYOffset()
                            + participant.y * distantRenderer.getScaledHeight())

                    var newWidth = Math.ceil(participant.w * distantRenderer.getScaledWidth())
                    var newHeight = Math.ceil(participant.h * distantRenderer.getScaledHeight())

                    var newVisible = participant.w !== 0 && participant.h !== 0
                    if (participantOverlays[p].x !== newX)
                        participantOverlays[p].x = newX
                    if (participantOverlays[p].y !== newY)
                        participantOverlays[p].y = newY
                    if (participantOverlays[p].width !== newWidth)
                        participantOverlays[p].width = newWidth
                    if (participantOverlays[p].height !== newHeight)
                        participantOverlays[p].height = newHeight
                    if (participantOverlays[p].visible !== newVisible)
                        participantOverlays[p].visible = newVisible

                    showMax = showMaximize(participantOverlays[p].x,
                                           participantOverlays[p].y,
                                           participantOverlays[p].width,
                                           participantOverlays[p].height)

                    participantOverlays[p].setMenu(participant.uri, participant.bestName,
                                                   participant.isLocal, participant.active, showMax)
                    if (participant.videoMuted)
                        participantOverlays[p].setAvatar(true, participant.avatar, participant.uri, participant.isLocal, participant.isContact)
                    else
                        participantOverlays[p].setAvatar(false)
                    currentUris.push(participantOverlays[p].uri)
                } else {
                    // Participant is no longer in conference
                    deletedUris.push(participantOverlays[p].uri)
                    participantOverlays[p].destroy()
                }
            }
        }
        participantOverlays = participantOverlays.filter(part => !deletedUris.includes(part.uri))

        if (infos.length === 0) { // Return to normal call
            previewRenderer.visible = true
            for (var part in participantOverlays) {
                if (participantOverlays[part]) {
                        participantOverlays[part].destroy()
                }
            }
            participantOverlays = []
        } else {
            previewRenderer.visible = false
            for (var infoVariant in infos) {
                // Only create overlay for new participants
                if (!currentUris.includes(infos[infoVariant].uri)) {
                    var hover = participantComponent.createObject(callOverlayRectMouseArea, {
                        x: Math.trunc(distantRenderer.getXOffset() + infos[infoVariant].x * distantRenderer.getScaledWidth()),
                        y: Math.trunc(distantRenderer.getYOffset() + infos[infoVariant].y * distantRenderer.getScaledHeight()),
                        width: Math.ceil(infos[infoVariant].w * distantRenderer.getScaledWidth()),
                        height: Math.ceil(infos[infoVariant].h * distantRenderer.getScaledHeight()),
                        visible: infos[infoVariant].w !== 0 && infos[infoVariant].h !== 0
                    })
                    if (!hover) {
                        console.log("Error when creating the hover")
                        return
                    }

                    showMax = showMaximize(hover.x, hover.y, hover.width, hover.height)

                    hover.setMenu(infos[infoVariant].uri, infos[infoVariant].bestName,
                                  infos[infoVariant].isLocal, infos[infoVariant].active, showMax)
                    if (infos[infoVariant].videoMuted)
                        hover.setAvatar(true, infos[infoVariant].avatar, infos[infoVariant].uri, infos[infoVariant].isLocal, infos[infoVariant].isContact)
                    else
                        hover.setAvatar(false)
                    participantOverlays.push(hover)
                }
            }
        }
    }

    // x, y position does not need to be translated
    // since they all fill the call page
    function openCallViewContextMenuInPos(x, y) {
        callViewContextMenu.x = x
        callViewContextMenu.y = y
        callViewContextMenu.openMenu()
    }

    function showRemoteRecording(peers, state) {
        var label = ""
        var i = 0
        if (state) {
            for (var p in peers) {
                label += peers[p]
                if (i !== (peers.length - 1))
                    label += ", "
                i += 1
            }
            label += " " + ((peers.length > 1)? JamiStrings.areRecording
                                              : JamiStrings.isRecording)
        }

        remoteRecordingLabel = state? label : JamiStrings.peerStoppedRecording
        callViewContextMenu.peerIsRecording = state
        recordingRect.visible = callViewContextMenu.localIsRecording
                || callViewContextMenu.peerIsRecording
        callOverlayRectMouseArea.entered()
    }

    function resetRemoteRecording() {
        remoteRecordingLabel = ""
        callViewContextMenu.peerIsRecording = false
        recordingRect.visible = callViewContextMenu.localIsRecording
    }

    anchors.fill: parent

    SipInputPanel {
        id: sipInputPanel

        x: root.width / 2 - sipInputPanel.width / 2
        y: root.height / 2 - sipInputPanel.height / 2
    }

    // Timer to decide when overlay fade out.
    Timer {
        id: callOverlayTimer
        interval: 5000
        onTriggered: {
            if (overlayUpperPartRect.state !== 'freezed') {
                overlayUpperPartRect.state = 'freezed'
                resetLabelsTimer.restart()
            }
            if (callOverlayButtonGroup.state !== 'freezed') {
                callOverlayButtonGroup.state = 'freezed'
                resetLabelsTimer.restart()
            }
        }
    }

    // Timer to reset recording label and call duration time
    Timer {
        id: resetLabelsTimer

        interval: 1000
        running: root.visible
        repeat: true
        onTriggered: {
            timeText = CallAdapter.getCallDurationTime(LRCInstance.currentAccountId,
                                                       LRCInstance.selectedConvUid)
            if (callOverlayButtonGroup.state === 'freezed'
                    && !callViewContextMenu.peerIsRecording)
                remoteRecordingLabel = ""
        }
    }

    Rectangle {
        id: overlayUpperPartRect

        anchors.top: root.top

        width: root.width
        height: 50
        opacity: 0

        RowLayout {
            id: overlayUpperPartRectRowLayout

            anchors.fill: parent

            Text {
                id: jamiBestNameText

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: overlayUpperPartRect.width / 3
                Layout.preferredHeight: 50
                leftPadding: 16

                font.pointSize: JamiTheme.textFontSize

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiBestNameText.elidedText
                color: "white"

                TextMetrics {
                    id: textMetricsjamiBestNameText
                    font: jamiBestNameText.font
                    text: {
                        if (!root.isAudioOnly) {
                            if (remoteRecordingLabel === "") {
                                return root.bestName
                            } else {
                                return remoteRecordingLabel
                            }
                        }
                        return ""
                    }
                    elideWidth: overlayUpperPartRect.width / 3
                    elide: Qt.ElideRight
                }
            }

            Text {
                id: callTimerText
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.preferredWidth: 64
                Layout.minimumWidth: 64
                Layout.preferredHeight: 48
                Layout.rightMargin: recordingRect.visible?
                                        0 : JamiTheme.preferredMarginSize
                font.pointSize: JamiTheme.textFontSize
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                text: textMetricscallTimerText.elidedText
                color: "white"
                TextMetrics {
                    id: textMetricscallTimerText
                    font: callTimerText.font
                    text: timeText
                    elideWidth: overlayUpperPartRect.width / 4
                    elide: Qt.ElideRight
                }
            }

            Rectangle {
                id: recordingRect
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.rightMargin: JamiTheme.preferredMarginSize
                height: 16
                width: 16
                radius: height / 2
                color: "red"

                SequentialAnimation on color {
                    loops: Animation.Infinite
                    running: true
                    ColorAnimation { from: "red"; to: "transparent";  duration: 500 }
                    ColorAnimation { from: "transparent"; to: "red"; duration: 500 }
                }
            }
        }

        color: "transparent"


        // Rect states: "entered" state should make overlay fade in,
        //              "freezed" state should make overlay fade out.
        // Combine with PropertyAnimation of opacity.
        states: [
            State {
                name: "entered"
                PropertyChanges {
                    target: overlayUpperPartRect
                    opacity: 1
                }
            },
            State {
                name: "freezed"
                PropertyChanges {
                    target: overlayUpperPartRect
                    opacity: 0
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation {
                target: overlayUpperPartRect
                property: "opacity"
                duration: 1000
            }
        }
    }

    ResponsiveImage {
        id: onHoldImage

        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter

        width: 200
        height: 200

        visible: false

        source: "qrc:/images/icons/ic_pause_white_100px.svg"
    }

    CallOverlayButtonGroup {
        id: callOverlayButtonGroup

        anchors.bottom: root.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: root.horizontalCenter

        height: 56
        width: root.width
        opacity: 0

        onChatButtonClicked: {
            root.overlayChatButtonClicked()
        }

        onAddToConferenceButtonClicked: {
            // Create contact picker - conference.
            ContactPickerCreation.createContactPickerObjects(
                        ContactList.CONFERENCE,
                        root)
            ContactPickerCreation.openContactPicker()
        }

        states: [
            State {
                name: "entered"
                PropertyChanges {
                    target: callOverlayButtonGroup
                    opacity: 1
                }
            },
            State {
                name: "freezed"
                PropertyChanges {
                    target: callOverlayButtonGroup
                    opacity: 0
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation {
                target: callOverlayButtonGroup
                property: "opacity"
                duration: 1000
            }
        }
    }

    // MouseAreas to make sure that overlay states are correctly set.
    MouseArea {
        id: callOverlayButtonGroupLeftSideMouseArea

        anchors.bottom: root.bottom
        anchors.left: root.left

        width: root.width / 6
        height: 60

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            callOverlayRectMouseArea.entered()
        }

        onMouseXChanged: {
            callOverlayRectMouseArea.entered()
        }
    }

    MouseArea {
        id: callOverlayButtonGroupRightSideMouseArea

        anchors.bottom: root.bottom
        anchors.right: root.right

        width: root.width / 6
        height: 60

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            callOverlayRectMouseArea.entered()
        }

        onMouseXChanged: {
            callOverlayRectMouseArea.entered()
        }
    }

    MouseArea {
        id: callOverlayRectMouseArea

        anchors.top: root.top

        width: root.width
        height: root.height

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton

        function resetStates() {
            if (overlayUpperPartRect.state !== 'entered') {
                overlayUpperPartRect.state = 'entered'
            }
            if (callOverlayButtonGroup.state !== 'entered') {
                callOverlayButtonGroup.state = 'entered'
            }
            callOverlayTimer.restart()
        }

        onReleased: {
            resetStates()
        }
        onEntered: {
            resetStates()
        }

        onMouseXChanged: {
            resetStates()
        }
    }

    color: "transparent"

    CallViewContextMenu {
        id: callViewContextMenu

        onTransferCallButtonClicked: {
            // Create contact picker - sip transfer.
            ContactPickerCreation.createContactPickerObjects(
                        ContactList.TRANSFER,
                        root)
            ContactPickerCreation.openContactPicker()
        }

        onPluginItemClicked: {
            // Create plugin handler picker - PLUGINS
            PluginHandlerPickerCreation.createPluginHandlerPickerObjects(root, true)
            PluginHandlerPickerCreation.openPluginHandlerPicker()
        }
    }
}
