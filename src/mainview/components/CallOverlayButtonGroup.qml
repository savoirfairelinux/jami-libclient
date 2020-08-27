
/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtQml 2.14
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    /*
     * ButtonCounts here is to make sure that flow layout margin is calculated correctly,
     * since no other methods can make buttons at the layout center.
     */
    property int buttonPreferredSize: 24
    property var isMaster: true
    property var isSip: false

    signal chatButtonClicked
    signal addToConferenceButtonClicked

    function updateMaster() {
        root.isMaster = CallAdapter.isCurrentMaster()
        addToConferenceButton.visible = !root.isSip && root.isMaster
    }

    function setButtonStatus(isPaused, isAudioOnly, isAudioMuted, isVideoMuted, isRecording, isSIP, isConferenceCall) {
        root.isMaster = CallAdapter.isCurrentMaster()
        root.isSip = isSIP
        noVideoButton.visible = !isAudioOnly
        addToConferenceButton.visible = !isSIP && isMaster

        noMicButton.checked = isAudioMuted
        noVideoButton.checked = isVideoMuted
    }

    color: "transparent"
    z: 2

    RowLayout {
        id: callOverlayButtonGroup

        spacing: 8
        height: 56

        anchors.fill: parent

        Item {
            Layout.preferredWidth: {
                // 6 is the number of button
                // If ~ 500px, go into wide mode
                (callOverlayButtonGroup.width < buttonPreferredSize * 12 - callOverlayButtonGroup.spacing * 6 + 300)?
                0 : callOverlayButtonGroup.width / 2 - buttonPreferredSize * 3 - callOverlayButtonGroup.spacing
            }
        }

        HoverableButton {
            id: noMicButton

            Layout.preferredWidth: buttonPreferredSize * 2
            Layout.preferredHeight: buttonPreferredSize * 2

            backgroundColor: Qt.rgba(0, 0, 0, 0.75)
            onEnterColor: Qt.rgba(0, 0, 0, 0.6)
            onPressColor: Qt.rgba(0, 0, 0, 0.5)
            onReleaseColor: Qt.rgba(0, 0, 0, 0.6)
            onExitColor: Qt.rgba(0, 0, 0, 0.75)

            buttonImageHeight: buttonPreferredSize
            buttonImageWidth: buttonPreferredSize
            baseImage: "qrc:/images/icons/ic_mic_white_24dp.png"
            checkedImage: "qrc:/images/icons/ic_mic_off_white_24dp.png"
            baseColor: "white"
            checkedColor: JamiTheme.declineButtonPressedRed
            radius: 30

            toolTipText: !checked ? qsTr("Press to mute the call") : qsTr("Press to unmute the call")

            onClicked: {
                CallAdapter.muteThisCallToggle()
            }
        }

        HoverableButton {
            id: hangUpButton

            Layout.preferredWidth: buttonPreferredSize * 2
            Layout.preferredHeight: buttonPreferredSize * 2

            backgroundColor: JamiTheme.declineButtonRed
            onEnterColor: JamiTheme.declineButtonHoverRed
            onPressColor: JamiTheme.declineButtonPressedRed
            onReleaseColor: JamiTheme.declineButtonHoverRed
            onExitColor: JamiTheme.declineButtonRed

            buttonImageHeight: buttonPreferredSize
            buttonImageWidth: buttonPreferredSize
            source: "qrc:/images/icons/ic_call_end_white_24px.svg"
            color: "white"
            radius: 30

            toolTipText: qsTr("Press to hang up the call")

            onClicked: {
                CallAdapter.hangUpThisCall()
            }
        }

        HoverableButton {
            id: noVideoButton

            Layout.preferredWidth: buttonPreferredSize * 2
            Layout.preferredHeight: buttonPreferredSize * 2

            backgroundColor: Qt.rgba(0, 0, 0, 0.75)
            onEnterColor: Qt.rgba(0, 0, 0, 0.6)
            onPressColor: Qt.rgba(0, 0, 0, 0.5)
            onReleaseColor: Qt.rgba(0, 0, 0, 0.6)
            onExitColor: Qt.rgba(0, 0, 0, 0.75)

            buttonImageHeight: buttonPreferredSize
            buttonImageWidth: buttonPreferredSize
            baseImage: "qrc:/images/icons/ic_videocam_white.png"
            checkedImage: "qrc:/images/icons/ic_videocam_off_white_24dp.png"
            baseColor: "white"
            checkedColor: JamiTheme.declineButtonPressedRed
            radius: 30

            toolTipText: !checked ? qsTr("Press to pause the call") : qsTr("Press to resume the call")

            onClicked: {
                CallAdapter.videoPauseThisCallToggle()
            }
        }

        Item {
            Layout.fillWidth: true
        }

        HoverableButton {
            id: addToConferenceButton

            Layout.preferredWidth: buttonPreferredSize * 2
            Layout.preferredHeight: buttonPreferredSize * 2
            visible: !isMaster

            backgroundColor: Qt.rgba(0, 0, 0, 0.75)
            onEnterColor: Qt.rgba(0, 0, 0, 0.6)
            onPressColor: Qt.rgba(0, 0, 0, 0.5)
            onReleaseColor: Qt.rgba(0, 0, 0, 0.6)
            onExitColor: Qt.rgba(0, 0, 0, 0.75)

            buttonImageHeight: buttonPreferredSize
            buttonImageWidth: buttonPreferredSize
            color: "white"
            source: "qrc:/images/icons/ic_group_add_white_24dp.png"
            radius: 30

            toolTipText: qsTr("Press to add more contact into conference call")

            onClicked: {
                root.addToConferenceButtonClicked()
            }
        }

        HoverableButton {
            id: chatButton

            Layout.preferredWidth: buttonPreferredSize * 2
            Layout.preferredHeight: buttonPreferredSize * 2

            backgroundColor: Qt.rgba(0, 0, 0, 0.75)
            onEnterColor: Qt.rgba(0, 0, 0, 0.6)
            onPressColor: Qt.rgba(0, 0, 0, 0.5)
            onReleaseColor: Qt.rgba(0, 0, 0, 0.6)
            onExitColor: Qt.rgba(0, 0, 0, 0.75)

            buttonImageHeight: buttonPreferredSize
            buttonImageWidth: buttonPreferredSize
            color: "white"
            source: "qrc:/images/icons/ic_chat_white_24dp.png"
            radius: 30

            toolTipText: qsTr("Press to toggle open the chatview")

            onClicked: {
                root.chatButtonClicked()
            }
        }

        HoverableButton {
            id: optionsButton

            Layout.preferredWidth: buttonPreferredSize * 2
            Layout.preferredHeight: buttonPreferredSize * 2

            backgroundColor: Qt.rgba(0, 0, 0, 0.75)
            onEnterColor: Qt.rgba(0, 0, 0, 0.6)
            onPressColor: Qt.rgba(0, 0, 0, 0.5)
            onReleaseColor: Qt.rgba(0, 0, 0, 0.6)
            onExitColor: Qt.rgba(0, 0, 0, 0.75)

            buttonImageHeight: buttonPreferredSize
            buttonImageWidth: buttonPreferredSize
            source: "qrc:/images/icons/more_vert-24px.svg"
            radius: 30

            toolTipText: qsTr("Press to open chat options")

            onClicked: {
                var rectPos = mapToItem(callStackViewWindow, optionsButton.x, optionsButton.y)
                callViewContextMenu.openMenu()
                callViewContextMenu.x = rectPos.x + optionsButton.width/2 - callViewContextMenu.width/2
                callViewContextMenu.y = rectPos.y - 12 - callViewContextMenu.height
            }
        }

        Item { Layout.preferredWidth: 8 }
    }
}
