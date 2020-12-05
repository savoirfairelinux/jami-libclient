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
import QtQuick.Controls.Universal 2.14
import QtQml 2.14
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    // ButtonCounts here is to make sure that flow layout margin is calculated correctly,
    // since no other methods can make buttons at the layout center.
    property int buttonPreferredSize: 48
    property var isHost: true
    property var isSip: false

    signal chatButtonClicked
    signal addToConferenceButtonClicked

    function updateMenu() {
        root.isHost = CallAdapter.isCurrentHost()
        addToConferenceButton.visible = !root.isSip && root.isHost
    }

    function setButtonStatus(isPaused, isAudioOnly, isAudioMuted, isVideoMuted, isRecording, isSIP, isConferenceCall) {
        root.isHost = CallAdapter.isCurrentModerator()
        root.isSip = isSIP
        noVideoButton.visible = !isAudioOnly
        addToConferenceButton.visible = !isSIP && isHost

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
                // TODO: refactor with Flow if possible
                // 6 is the number of button
                // If ~ 500px, go into wide mode
                if (callOverlayButtonGroup.width < (buttonPreferredSize * 6 -
                        callOverlayButtonGroup.spacing * 6 + 300)) {
                    return 0
                } else {
                    return  callOverlayButtonGroup.width / 2 - buttonPreferredSize * 1.5 -
                            callOverlayButtonGroup.spacing
                }
            }
        }

        PushButton {
            id: noMicButton

            Layout.leftMargin: 8
            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize

            pressedColor: JamiTheme.invertedPressedButtonColor
            hoveredColor: JamiTheme.invertedHoveredButtonColor
            normalColor: JamiTheme.invertedNormalButtonColor

            normalImageSource: "qrc:/images/icons/mic-24px.svg"
            imageColor: JamiTheme.whiteColor
            checkable: true
            checkedImageSource: "qrc:/images/icons/mic_off-24px.svg"
            checkedImageColor: JamiTheme.declineButtonPressedRed

            toolTipText: !checked ? JamiStrings.mute : JamiStrings.unmute

            onClicked: CallAdapter.muteThisCallToggle()
        }

        PushButton {
            id: hangUpButton

            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize

            pressedColor: JamiTheme.declineButtonPressedRed
            hoveredColor: JamiTheme.declineButtonHoverRed
            normalColor: JamiTheme.declineButtonRed

            source: "qrc:/images/icons/ic_call_end_white_24px.svg"
            imageColor: JamiTheme.whiteColor

            toolTipText: JamiStrings.hangup

            onClicked: CallAdapter.hangUpThisCall()
        }

        PushButton {
            id: noVideoButton

            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize

            pressedColor: JamiTheme.invertedPressedButtonColor
            hoveredColor: JamiTheme.invertedHoveredButtonColor
            normalColor: JamiTheme.invertedNormalButtonColor

            normalImageSource: "qrc:/images/icons/videocam-24px.svg"
            imageColor: JamiTheme.whiteColor
            checkable: true
            checkedImageSource: "qrc:/images/icons/videocam_off-24px.svg"
            checkedImageColor: JamiTheme.declineButtonPressedRed

            toolTipText: !checked ? JamiStrings.pause : JamiStrings.resume

            onClicked: CallAdapter.videoPauseThisCallToggle()
        }

        Item {
            Layout.fillWidth: true
        }

        PushButton {
            id: addToConferenceButton

            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize
            visible: !isHost

            pressedColor: JamiTheme.invertedPressedButtonColor
            hoveredColor: JamiTheme.invertedHoveredButtonColor
            normalColor: JamiTheme.invertedNormalButtonColor

            source: "qrc:/images/icons/group_add-24px.svg"
            imageColor: JamiTheme.whiteColor

            toolTipText: JamiStrings.addParticipants

            onClicked: root.addToConferenceButtonClicked()
        }

        PushButton {
            id: chatButton

            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize

            pressedColor: JamiTheme.invertedPressedButtonColor
            hoveredColor: JamiTheme.invertedHoveredButtonColor
            normalColor: JamiTheme.invertedNormalButtonColor

            source: "qrc:/images/icons/chat-24px.svg"
            imageColor: JamiTheme.whiteColor

            toolTipText: JamiStrings.chat

            onClicked: root.chatButtonClicked()
        }

        PushButton {
            id: optionsButton

            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize
            Layout.rightMargin: 8

            pressedColor: JamiTheme.invertedPressedButtonColor
            hoveredColor: JamiTheme.invertedHoveredButtonColor
            normalColor: JamiTheme.invertedNormalButtonColor

            source: "qrc:/images/icons/more_vert-24px.svg"
            imageColor: JamiTheme.whiteColor

            toolTipText: JamiStrings.moreOptions

            onClicked: {
                var rectPos = mapToItem(callStackViewWindow, optionsButton.x, optionsButton.y)
                callViewContextMenu.x = rectPos.x + optionsButton.width / 2
                        - callViewContextMenu.width / 2
                callViewContextMenu.y = rectPos.y - 12 - callViewContextMenu.height
                callViewContextMenu.openMenu()
            }
        }
    }
}
