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
import "../js/mediahandlerpickercreation.js" as MediaHandlerPickerCreation

import "../../commoncomponents"

Rectangle {
    id: callOverlayRect

    property string timeText: "00:00"

    signal overlayChatButtonClicked

    property var participantOverlays: []
    property var participantComponent: Qt.createComponent("ParticipantOverlay.qml")

    function setRecording(isRecording) {
        callViewContextMenu.isRecording = isRecording
        recordingRect.visible = isRecording
    }

    function updateButtonStatus(isPaused, isAudioOnly, isAudioMuted, isVideoMuted,
                                isRecording, isSIP, isConferenceCall) {
        callViewContextMenu.isSIP = isSIP
        callViewContextMenu.isPaused = isPaused
        callViewContextMenu.isAudioOnly = isAudioOnly
        callViewContextMenu.isRecording = isRecording
        recordingRect.visible = isRecording
        callOverlayButtonGroup.setButtonStatus(isPaused, isAudioOnly,
                                               isAudioMuted, isVideoMuted,
                                               isRecording, isSIP,
                                               isConferenceCall)
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

    function closePotentialMediaHandlerPicker() {
        MediaHandlerPickerCreation.closeMediaHandlerPicker()
    }

    // returns true if participant is not fully maximized
    function showMaximize(pX, pY, pW, pH) {
        // Hack: -1 offset added to avoid problems with odd sizes
        return (pX - distantRenderer.getXOffset() !== 0
                || pY - distantRenderer.getYOffset() !== 0
                || pW < (distantRenderer.width - distantRenderer.getXOffset() * 2 - 1)
                || pH < (distantRenderer.height - distantRenderer.getYOffset() * 2 - 1))
    }

    // returns true if participant takes renderer's width
    function showMinimize(pX, pW) {
        return (pX - distantRenderer.getXOffset() === 0
                && pW >= distantRenderer.width - distantRenderer.getXOffset() * 2 - 1)
    }


    function handleParticipantsInfo(infos) {
        // TODO: in the future the conference layout should be entirely managed by the client
        videoCallOverlay.updateMenu()
        var showMax = false
        var showMin = false

        var deletedUris = []
        var currentUris = []
        for (var p in participantOverlays) {
            if (participantOverlays[p]) {
                var participant = infos.find(e => e.uri === participantOverlays[p].uri);
                if (participant) {
                    // Update participant's information
                    var newX = distantRenderer.getXOffset()
                            + participant.x * distantRenderer.getScaledWidth()
                    var newY = distantRenderer.getYOffset()
                            + participant.y * distantRenderer.getScaledHeight()
                    var newWidth = participant.w * distantRenderer.getScaledWidth()
                    var newHeight = participant.h * distantRenderer.getScaledHeight()
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
                    showMin = showMinimize(participantOverlays[p].x,
                                           participantOverlays[p].width)

                    participantOverlays[p].setMenu(participant.uri, participant.bestName,
                                                   participant.isLocal, showMax, showMin)
                    if (participant.videoMuted)
                        participantOverlays[p].setAvatar(participant.avatar)
                    else
                        participantOverlays[p].setAvatar("")
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
                        x: distantRenderer.getXOffset() + infos[infoVariant].x * distantRenderer.getScaledWidth(),
                        y: distantRenderer.getYOffset() + infos[infoVariant].y * distantRenderer.getScaledHeight(),
                        width: infos[infoVariant].w * distantRenderer.getScaledWidth(),
                        height: infos[infoVariant].h * distantRenderer.getScaledHeight(),
                        visible: infos[infoVariant].w !== 0 && infos[infoVariant].h !== 0
                    })
                    if (!hover) {
                        console.log("Error when creating the hover")
                        return
                    }

                    showMax = showMaximize(hover.x, hover.y, hover.width, hover.height)
                    showMin = showMinimize(hover.x, hover.width)

                    hover.setMenu(infos[infoVariant].uri, infos[infoVariant].bestName,
                                  infos[infoVariant].isLocal, showMax, showMin)
                    if (infos[infoVariant].videoMuted)
                        hover.setAvatar(infos[infoVariant].avatar)
                    else
                        hover.setAvatar("")
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

    anchors.fill: parent

    SipInputPanel {
        id: sipInputPanel

        x: callOverlayRect.width / 2 - sipInputPanel.width / 2
        y: callOverlayRect.height / 2 - sipInputPanel.height / 2
    }

    // Timer to decide when overlay fade out.
    Timer {
        id: callOverlayTimer
        interval: 5000
        onTriggered: {
            if (overlayUpperPartRect.state !== 'freezed') {
                overlayUpperPartRect.state = 'freezed'
            }
            if (callOverlayButtonGroup.state !== 'freezed') {
                callOverlayButtonGroup.state = 'freezed'
            }
        }
    }

    Rectangle {
        id: overlayUpperPartRect

        anchors.top: callOverlayRect.top

        width: callOverlayRect.width
        height: 50
        opacity: 0

        RowLayout {
            id: overlayUpperPartRectRowLayout

            anchors.fill: parent

            PushButton {
                id: backButton

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.topMargin: JamiTheme.preferredMarginSize
                Layout.leftMargin: JamiTheme.preferredMarginSize


                source: JamiQmlUtils.callIsFullscreen ?
                            "qrc:/images/icons/round-close-24px.svg" :
                            "qrc:/images/icons/ic_arrow_back_24px.svg"

                pressedColor: JamiTheme.invertedPressedButtonColor
                hoveredColor: JamiTheme.invertedHoveredButtonColor
                normalColor: JamiTheme.invertedNormalButtonColor

                imageColor: JamiTheme.whiteColor

                toolTipText: qsTr("Toggle to display side panel")

                visible: true

                onClicked: {
                    if (JamiQmlUtils.callIsFullscreen) {
                        callStackView.toggleFullScreen()
                    } else {
                        mainViewWindow.showWelcomeView()
                    }
                }
            }

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
                        if (videoCallPageRect)
                            return videoCallPageRect.bestName
                        return ""
                    }
                    elideWidth: overlayUpperPartRect.width / 3
                    elide: Qt.ElideRight
                }
            }

            Text {
                id: callTimerText
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.preferredWidth: overlayUpperPartRect.width / 3
                Layout.preferredHeight: 48
                font.pointSize: JamiTheme.textFontSize
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                text: textMetricscallTimerText.elidedText
                color: "white"
                TextMetrics {
                    id: textMetricscallTimerText
                    font: callTimerText.font
                    text: timeText
                    elideWidth: overlayUpperPartRect.width / 3
                    elide: Qt.ElideRight
                }
            }

            Rectangle {
                id: recordingRect
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
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

            Item {
                width: 8
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

        anchors.verticalCenter: callOverlayRect.verticalCenter
        anchors.horizontalCenter: callOverlayRect.horizontalCenter

        width: 200
        height: 200

        visible: false

        source: "qrc:/images/icons/phone_paused-24px.svg"
    }

    CallOverlayButtonGroup {
        id: callOverlayButtonGroup

        anchors.bottom: callOverlayRect.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: callOverlayRect.horizontalCenter

        height: 56
        width: callOverlayRect.width
        opacity: 0

        onChatButtonClicked: {
            callOverlayRect.overlayChatButtonClicked()
        }

        onAddToConferenceButtonClicked: {
            // Create contact picker - conference.
            ContactPickerCreation.createContactPickerObjects(
                        ContactPicker.ContactPickerType.JAMICONFERENCE,
                        callOverlayRect)
            ContactPickerCreation.calculateCurrentGeo(
                        callOverlayRect.width / 2, callOverlayRect.height / 2)
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

        anchors.bottom: callOverlayRect.bottom
        anchors.left: callOverlayRect.left

        width: callOverlayRect.width / 6
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

        anchors.bottom: callOverlayRect.bottom
        anchors.right: callOverlayRect.right

        width: callOverlayRect.width / 6
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

        anchors.top: callOverlayRect.top

        width: callOverlayRect.width
        height: callOverlayRect.height

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            if (overlayUpperPartRect.state !== 'entered') {
                overlayUpperPartRect.state = 'entered'
            }
            if (callOverlayButtonGroup.state !== 'entered') {
                callOverlayButtonGroup.state = 'entered'
            }
            callOverlayTimer.restart()
        }

        onMouseXChanged: {
            if (overlayUpperPartRect.state !== 'entered') {
                overlayUpperPartRect.state = 'entered'
            }
            if (callOverlayButtonGroup.state !== 'entered') {
                callOverlayButtonGroup.state = 'entered'
            }
            callOverlayTimer.restart()
        }
    }

    color: "transparent"

    onWidthChanged: {
        ContactPickerCreation.calculateCurrentGeo(callOverlayRect.width / 2,
                                                  callOverlayRect.height / 2)
        MediaHandlerPickerCreation.calculateCurrentGeo(callOverlayRect.width / 2,
                                                  callOverlayRect.height / 2)
    }

    onHeightChanged: {
        ContactPickerCreation.calculateCurrentGeo(callOverlayRect.width / 2,
                                                  callOverlayRect.height / 2)
        MediaHandlerPickerCreation.calculateCurrentGeo(callOverlayRect.width / 2,
                                                  callOverlayRect.height / 2)
    }

    CallViewContextMenu {
        id: callViewContextMenu

        onTransferCallButtonClicked: {
            // Create contact picker - sip transfer.
            ContactPickerCreation.createContactPickerObjects(
                        ContactPicker.ContactPickerType.SIPTRANSFER,
                        callOverlayRect)
            ContactPickerCreation.calculateCurrentGeo(
                        callOverlayRect.width / 2, callOverlayRect.height / 2)
            ContactPickerCreation.openContactPicker()
        }

        onPluginItemClicked: {
            // Create media handler picker - PLUGINS
            MediaHandlerPickerCreation.createMediaHandlerPickerObjects(callOverlayRect)
            MediaHandlerPickerCreation.calculateCurrentGeo(
                        callOverlayRect.width / 2, callOverlayRect.height / 2)
            MediaHandlerPickerCreation.openMediaHandlerPicker()
        }
    }
}
