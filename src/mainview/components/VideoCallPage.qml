
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
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: videoCallPageRect

    property string bestName: "Best Name"
    property string bestId: "Best Id"
    property variant clickPos: "1,1"
    property int previewMargin: 15
    property int previewToX: 0
    property int previewToY: 0

    property var corrspondingMessageWebView: null

    signal videoCallPageBackButtonIsClicked
    signal needToShowInFullScreen

    function updateUI(accountId, convUid) {
        bestName = ClientWrapper.utilsAdaptor.getBestName(accountId, convUid)

        var id = ClientWrapper.utilsAdaptor.getBestId(accountId, convUid)
        bestId = (bestName !== id) ? id : ""
    }

    function setDistantRendererId(id) {
        distantRenderer.setRendererId(id)
    }

    function setVideoCallPageCorrspondingMessageWebView(webViewId) {
        corrspondingMessageWebView = webViewId
        corrspondingMessageWebView.needToHideConversationInCall.disconnect(
                    closeInCallConversation)
        corrspondingMessageWebView.needToHideConversationInCall.connect(
                    closeInCallConversation)
    }

    function closeInCallConversation() {
        if (inVideoCallMessageWebViewStack.visible) {
            corrspondingMessageWebView.resetMessagingHeaderBackButtonSource(
                        true)
            corrspondingMessageWebView.setMessagingHeaderButtonsVisible(true)
            inVideoCallMessageWebViewStack.visible = false
            inVideoCallMessageWebViewStack.clear()
        }
    }

    function closeContextMenuAndRelatedWindows() {
        videoCallPageContextMenu.closePotentialWindows()
        videoCallPageContextMenu.close()
        videoCallOverlay.closePotentialContactPicker()
    }

    function previewMagneticSnap() {


        /*
         * Calculate the position where the previewRenderer should attach to.
         */
        var previewRendererCenter = Qt.point(
                    previewRenderer.x + previewRenderer.width / 2,
                    previewRenderer.y + previewRenderer.height / 2)
        var distantRendererCenter = Qt.point(
                    distantRenderer.x + distantRenderer.width / 2,
                    distantRenderer.y + distantRenderer.height / 2)

        if (previewRendererCenter.x >= distantRendererCenter.x) {
            if (previewRendererCenter.y >= distantRendererCenter.y) {


                /*
                 * Bottom right.
                 */
                previewToX = Qt.binding(function () {
                    return videoCallPageMainRect.width - previewRenderer.width - previewMargin
                })
                previewToY = Qt.binding(function () {
                    return videoCallPageMainRect.height - previewRenderer.height - previewMargin
                })
            } else {


                /*
                 * Top right.
                 */
                previewToX = Qt.binding(function () {
                    return videoCallPageMainRect.width - previewRenderer.width - previewMargin
                })
                previewToY = previewMargin
            }
        } else {
            if (previewRendererCenter.y >= distantRendererCenter.y) {


                /*
                 * Bottom left.
                 */
                previewToX = previewMargin
                previewToY = Qt.binding(function () {
                    return videoCallPageMainRect.height - previewRenderer.height - previewMargin
                })
            } else {


                /*
                 * Top left.
                 */
                previewToX = previewMargin
                previewToY = previewMargin
            }
        }
        previewRenderer.state = "geoChanging"
    }

    function setCallOverlayBackButtonVisible(visible) {
        videoCallOverlay.setBackTintedButtonVisible(visible)
    }

    anchors.fill: parent

    SplitView {
        id: mainColumnLayout

        anchors.fill: parent

        orientation: Qt.Vertical

        handle: Rectangle {
            implicitWidth: videoCallPageRect.width
            implicitHeight: JamiTheme.splitViewHandlePreferedWidth
            color: SplitHandle.pressed ? JamiTheme.pressColor : (SplitHandle.hovered ? JamiTheme.hoverColor : JamiTheme.tabbarBorderColor)
        }

        Rectangle {
            id: videoCallPageMainRect

            SplitView.preferredHeight: (videoCallPageRect.height / 3) * 2
            SplitView.minimumHeight: videoCallPageRect.height / 2 + 20
            SplitView.fillWidth: true

            CallOverlay {
                id: videoCallOverlay

                anchors.fill: parent

                Connections {
                    target: CallAdapter

                    function onUpdateTimeText(time) {
                        videoCallOverlay.timeText = time
                    }

                    function onUpdateOverlay(isPaused, isAudioOnly, isAudioMuted, isVideoMuted, isRecording, isSIP, isConferenceCall, bestName) {
                        videoCallOverlay.showOnHoldImage(isPaused)
                        videoCallOverlay.updateButtonStatus(isPaused,
                                                            isAudioOnly,
                                                            isAudioMuted,
                                                            isVideoMuted,
                                                            isRecording, isSIP,
                                                            isConferenceCall)
                        videoCallOverlay.bestName = bestName
                    }

                    function onShowOnHoldLabel(isPaused) {
                        videoCallOverlay.showOnHoldImage(isPaused)
                    }
                }

                onBackButtonIsClicked: {
                    if (inVideoCallMessageWebViewStack.visible) {
                        corrspondingMessageWebView.resetMessagingHeaderBackButtonSource(
                                    true)
                        corrspondingMessageWebView.setMessagingHeaderButtonsVisible(
                                    true)
                        inVideoCallMessageWebViewStack.visible = false
                        inVideoCallMessageWebViewStack.clear()
                    }
                    videoCallPageRect.videoCallPageBackButtonIsClicked()
                }

                onOverlayChatButtonClicked: {
                    if (inVideoCallMessageWebViewStack.visible) {
                        corrspondingMessageWebView.resetMessagingHeaderBackButtonSource(
                                    true)
                        corrspondingMessageWebView.setMessagingHeaderButtonsVisible(
                                    true)
                        inVideoCallMessageWebViewStack.visible = false
                        inVideoCallMessageWebViewStack.clear()
                    } else {
                        corrspondingMessageWebView.resetMessagingHeaderBackButtonSource(
                                    false)
                        corrspondingMessageWebView.setMessagingHeaderButtonsVisible(
                                    false)
                        inVideoCallMessageWebViewStack.visible = true
                        inVideoCallMessageWebViewStack.push(
                                    corrspondingMessageWebView)
                    }
                }
            }

            DistantRenderer {
                id: distantRenderer

                anchors.centerIn: videoCallPageMainRect
                z: -1

                width: videoCallPageMainRect.width
                height: videoCallPageMainRect.height
            }

            VideoCallPreviewRenderer {
                id: previewRenderer


                /*
                 * Property is used in the {} expression for height (extra dependency),
                 * it will not affect the true height expression, since expression
                 * at last will be taken only, but it will force the height to update
                 * and reevaluate getPreviewImageScalingFactor().
                 */
                property int previewImageScalingFactorUpdated: 0

                Connections {
                    target: CallAdapter

                    function onPreviewVisibilityNeedToChange(visible) {
                        previewRenderer.visible = visible
                    }
                }

                width: videoCallPageMainRect.width / 4
                height: {
                    previewImageScalingFactorUpdated
                    return previewRenderer.width * previewRenderer.getPreviewImageScalingFactor()
                }
                x: videoCallPageMainRect.width - previewRenderer.width - previewMargin
                y: videoCallPageMainRect.height - previewRenderer.height - previewMargin
                z: -1

                states: [
                    State {
                        name: "geoChanging"
                        PropertyChanges {
                            target: previewRenderer
                            x: previewToX
                            y: previewToY
                        }
                    }
                ]

                transitions: Transition {
                    PropertyAnimation {
                        properties: "x,y"
                        easing.type: Easing.OutExpo
                        duration: 250

                        onStopped: {
                            previewRenderer.state = ""
                        }
                    }
                }

                MouseArea {
                    id: dragMouseArea

                    anchors.fill: previewRenderer

                    onPressed: {
                        clickPos = Qt.point(mouse.x, mouse.y)
                    }

                    onReleased: {
                        previewRenderer.state = ""
                        previewMagneticSnap()
                    }

                    onPositionChanged: {


                        /*
                         * Calculate mouse position relative change.
                         */
                        var delta = Qt.point(mouse.x - clickPos.x,
                                             mouse.y - clickPos.y)
                        var deltaW = previewRenderer.x + delta.x + previewRenderer.width
                        var deltaH = previewRenderer.y + delta.y + previewRenderer.height


                        /*
                         * Check if the previewRenderer exceeds the border of videoCallPageMainRect.
                         */
                        if (deltaW < videoCallPageMainRect.width
                                && previewRenderer.x + delta.x > 1)
                            previewRenderer.x += delta.x
                        if (deltaH < videoCallPageMainRect.height
                                && previewRenderer.y + delta.y > 1)
                            previewRenderer.y += delta.y
                    }
                }

                onPreviewImageAvailable: {
                    previewImageScalingFactorUpdated++
                    previewImageScalingFactorUpdated--
                }
            }

            color: "transparent"
        }

        StackView {
            id: inVideoCallMessageWebViewStack

            SplitView.preferredHeight: videoCallPageRect.height / 3
            SplitView.fillWidth: true

            visible: false

            clip: true
        }
    }

    VideoCallPageContextMenu {
        id: videoCallPageContextMenu

        onFullScreenNeeded: {
            videoCallPageRect.needToShowInFullScreen()
        }
    }

    MouseArea {
        anchors.fill: parent

        propagateComposedEvents: true
        acceptedButtons: Qt.RightButton

        onClicked: {


            /*
             * Make menu pos at mouse.
             */
            var relativeMousePos = mapToItem(videoCallPageRect,
                                             mouse.x, mouse.y)
            videoCallPageContextMenu.x = relativeMousePos.x
            videoCallPageContextMenu.y = relativeMousePos.y
            videoCallPageContextMenu.activate()
        }
    }

    color: "black"
}
