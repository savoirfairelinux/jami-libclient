/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

import QtQuick 2.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../js/contactpickercreation.js" as ContactPickerCreation
import "../js/selectscreenwindowcreation.js" as SelectScreenWindowCreation
import "../js/screenrubberbandcreation.js" as ScreenRubberBandCreation
import "../js/pluginhandlerpickercreation.js" as PluginHandlerPickerCreation

import "../../commoncomponents"

Item {
    id: root

    property alias participantsLayer: __participantsLayer

    property bool isPaused
    property bool isAudioOnly
    property bool isAudioMuted
    property bool isVideoMuted
    property bool isRecording
    property bool isSIP
    property bool isModerator
    property bool isConferenceCall
    property bool isGrid
    property bool localHandRaised

    signal chatButtonClicked

    ParticipantsLayer {
        id: __participantsLayer
        visible: !root.isAudioOnly
        anchors.fill: parent
    }

    function setRecording(localIsRecording) {
        callViewContextMenu.localIsRecording = localIsRecording
        mainOverlay.recordingVisible = localIsRecording
                || callViewContextMenu.peerIsRecording
    }

    function updateUI(isPaused, isAudioOnly, isAudioMuted,
                      isVideoMuted, isSIP,
                      isConferenceCall, isGrid) {
        if (isPaused !== undefined) {
            root.isPaused = isPaused
            root.isAudioOnly = isAudioOnly
            root.isAudioMuted = isAudioMuted
            root.isVideoMuted = isVideoMuted
            callViewContextMenu.isVideoMuted = root.isVideoMuted
            root.isSIP = isSIP
            root.isConferenceCall = isConferenceCall
            root.isGrid = isGrid
            mainOverlay.recordingVisible = isRecording
            root.localHandRaised = CallAdapter.isHandRaised()
        }
        root.isRecording = CallAdapter.isRecordingThisCall()
        root.isModerator = CallAdapter.isCurrentModerator()
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
            label += " " + ((peers.length > 1) ? JamiStrings.areRecording : JamiStrings.isRecording)
        }

        mainOverlay.remoteRecordingLabel = state ? label : JamiStrings.peerStoppedRecording
        callViewContextMenu.peerIsRecording = state
        mainOverlay.recordingVisible = callViewContextMenu.localIsRecording
                || callViewContextMenu.peerIsRecording
        callOverlayRectMouseArea.entered()
    }

    function resetRemoteRecording() {
        mainOverlay.remoteRecordingLabel = ""
        callViewContextMenu.peerIsRecording = false
        mainOverlay.recordingVisible = callViewContextMenu.localIsRecording
    }

    SipInputPanel {
        id: sipInputPanel

        x: root.width / 2 - sipInputPanel.width / 2
        y: root.height / 2 - sipInputPanel.height / 2
    }

    JamiFileDialog {
        id: jamiFileDialog

        mode: JamiFileDialog.Mode.OpenFile

        onAccepted: {
            if (AvAdapter.currentRenderingDeviceType !== Video.DeviceType.DISPLAY && AvAdapter.currentRenderingDeviceType !== Video.DeviceType.FILE) {
                AvAdapter.muteCamera = root.isVideoMuted
            }
            AvAdapter.shareFile(jamiFileDialog.file)
        }
    }

    ResponsiveImage {
        id: onHoldImage

        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter

        width: 200
        height: 200

        visible: false

        source: JamiResources.ic_pause_white_100px_svg
    }

    function openContactPicker(type) {
        ContactPickerCreation.createContactPickerObjects(type, root)
        ContactPickerCreation.openContactPicker()
    }

    function openShareScreen() {
        if (AvAdapter.currentRenderingDeviceType !== Video.DeviceType.DISPLAY && AvAdapter.currentRenderingDeviceType !== Video.DeviceType.FILE) {
            AvAdapter.muteCamera = root.isVideoMuted
        }
        AvAdapter.getListWindows()
        if (Qt.application.screens.length + AvAdapter.windowsNames.length === 1) {
            AvAdapter.shareEntireScreen(0)
        } else {
            SelectScreenWindowCreation.createSelectScreenWindowObject()
            SelectScreenWindowCreation.showSelectScreenWindow(callPreviewId)
        }
    }

    function openShareScreenArea() {
        if (AvAdapter.currentRenderingDeviceType !== Video.DeviceType.DISPLAY && AvAdapter.currentRenderingDeviceType !== Video.DeviceType.FILE) {
            AvAdapter.muteCamera = root.isVideoMuted
        }
        if (Qt.platform.os !== "windows") {
            AvAdapter.shareScreenArea(0, 0, 0, 0)
        } else {
            ScreenRubberBandCreation.createScreenRubberBandWindowObject()
            ScreenRubberBandCreation.showScreenRubberBandWindow()
        }
    }

    function openPluginsMenu() {
        PluginHandlerPickerCreation.createPluginHandlerPickerObjects(root, true)
        PluginHandlerPickerCreation.openPluginHandlerPicker()
    }

    MainOverlay {
        id: mainOverlay

        anchors.fill: parent

        Connections {
            target: mainOverlay.callActionBar
            function onChatClicked() { root.chatButtonClicked() }
            function onAddToConferenceClicked() { openContactPicker(ContactList.CONFERENCE) }
            function onTransferClicked() { openContactPicker(ContactList.TRANSFER) }
            function onResumePauseCallClicked() { CallAdapter.holdThisCallToggle() }
            function onShowInputPanelClicked() { sipInputPanel.open() }
            function onShareScreenClicked() { openShareScreen() }
            function onStopSharingClicked() { AvAdapter.stopSharing() }
            function onShareScreenAreaClicked() { openShareScreenArea() }
            function onShareFileClicked() { jamiFileDialog.open() }
            function onPluginsClicked() { openPluginsMenu() }
        }
    }

    CallViewContextMenu {
        id: callViewContextMenu

        isSIP: root.isSIP
        isPaused: root.isPaused
        localIsRecording: root.isRecording

        onTransferCallButtonClicked: openContactPicker(ContactList.TRANSFER)
        onPluginItemClicked: openPluginsMenu()
    }
}
