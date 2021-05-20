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

Item {
    id: root

    property alias participantsLayer: __participantsLayer
    property string timeText: "00:00"
    property string remoteRecordingLabel: ""
    property bool isVideoMuted: true
    property bool isAudioOnly: false
    property string bestName: ""

    signal overlayChatButtonClicked

    onVisibleChanged: if (!visible) callViewContextMenu.close()

    ParticipantsLayer {
        id: __participantsLayer
        anchors.fill: parent
    }

    function setRecording(localIsRecording) {
        callViewContextMenu.localIsRecording = localIsRecording
        recordingRect.visible = localIsRecording
                || callViewContextMenu.peerIsRecording
    }

    function updateButtonStatus(isPaused, isAudioOnly, isAudioMuted, isVideoMuted,
                                isRecording, isSIP, isConferenceCall) {
        root.isVideoMuted = isVideoMuted
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

    SipInputPanel {
        id: sipInputPanel

        x: root.width / 2 - sipInputPanel.width / 2
        y: root.height / 2 - sipInputPanel.height / 2
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

    Item {
        id: mainOverlay

        anchors.fill: parent
        opacity: 0

        // (un)subscribe to an app-wide mouse move event trap filtered
        // for the overlay's geometry
        onVisibleChanged: visible ?
                              CallOverlayModel.registerFilter(appWindow, this) :
                              CallOverlayModel.unregisterFilter(appWindow, this)

        Connections {
            target: CallOverlayModel

            function onMouseMoved(item) {
                if (item === mainOverlay) {
                    mainOverlay.opacity = 1
                    fadeOutTimer.restart()
                }
            }
        }

        // control overlay fade out.
        Timer {
            id: fadeOutTimer
            interval: JamiTheme.overlayFadeDelay
            onTriggered: {
                if (callOverlayButtonGroup.hovered)
                    return
                mainOverlay.opacity = 0
                resetLabelsTimer.restart()
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
                if (mainOverlay.opacity === 0 && !callViewContextMenu.peerIsRecording)
                    remoteRecordingLabel = ""
            }
        }

        Item {
            id: overlayUpperPartRect

            anchors.top: parent.top

            width: parent.width
            height: 50

            RowLayout {
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
        }

        CallOverlayButtonGroup {
            id: callOverlayButtonGroup

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter

            height: 56
            width: root.width

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
        }

        Behavior on opacity { NumberAnimation { duration: JamiTheme.overlayFadeDuration }}
    }

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
