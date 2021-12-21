/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

import QtQuick
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Item {
    id: root

    property string timeText: "00:00"
    property string remoteRecordingLabel: ""

    property alias callActionBar: __callActionBar

    property bool frozen: callActionBar.overflowOpen ||
                          callActionBar.hovered ||
                          callActionBar.subMenuOpen ||
                          participantCallInStatusView.visible

    property string muteAlertMessage: ""
    property bool muteAlertActive: false
    property bool remoteRecording: false
    property bool isRecording: false

    onMuteAlertActiveChanged: {
        if (muteAlertActive) {
            alertTimer.restart()
        }
    }

    opacity: 0

    // (un)subscribe to an app-wide mouse move event trap filtered
    // for the overlay's geometry
    onVisibleChanged: visible ? CallOverlayModel.registerFilter(
                                    appWindow,
                                    this) : CallOverlayModel.unregisterFilter(
                                    appWindow, this)

    Connections {
        target: CallOverlayModel

        function onMouseMoved(item) {
            if (item === root) {
                root.opacity = 1
                fadeOutTimer.restart()
            }
        }
    }

    // control overlay fade out.
    Timer {
        id: fadeOutTimer
        interval: JamiTheme.overlayFadeDelay
        onTriggered: {
            if (frozen)
                return
            root.opacity = 0
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
            root.timeText = CallAdapter.getCallDurationTime(
                        LRCInstance.currentAccountId,
                        LRCInstance.selectedConvUid)
            if (root.opacity === 0 && !root.remoteRecording)
                root.remoteRecordingLabel = ""
        }
    }

    Item {
        id: overlayUpperPartRect

        anchors.top: parent.top

        width: parent.width
        height: 50

        RowLayout {
            anchors.fill: parent

            spacing: 0

            Text {
                id: jamiBestNameText

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: overlayUpperPartRect.width / 2
                Layout.preferredHeight: 50

                leftPadding: 16

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                font.pointSize: JamiTheme.textFontSize
                text: {
                    if (!root.isAudioOnly) {
                        if (remoteRecordingLabel === "") {
                            return CurrentConversation.title
                        } else {
                            return remoteRecordingLabel
                        }
                    }
                    return ""
                }
                color: JamiTheme.whiteColor
                elide: Qt.ElideRight
            }

            PushButton {
                id: mosaicButton

                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.preferredWidth: JamiTheme.mosaicButtonPreferredWidth
                Layout.preferredHeight: 30
                Layout.rightMargin: 5

                visible: isConferenceCall && !isGrid

                preferredMargin: JamiTheme.mosaicButtonPreferredMargin
                radius: JamiTheme.mosaicButtonRadius
                opacity: JamiTheme.mosaicButtonOpacity

                buttonText: JamiStrings.mosaic
                buttonTextColor: JamiTheme.whiteColor
                buttonTextHeight: JamiTheme.mosaicButtonTextPreferredHeight
                buttonTextFont.weight: Font.DemiBold
                buttonTextFont.pointSize: JamiTheme.mosaicButtonTextPointSize
                textHAlign: Text.AlignLeft

                imageColor: JamiTheme.whiteColor
                imageContainerHeight: 20
                imageContainerWidth: 20
                source: JamiResources.mosaic_black_24dp_svg

                normalColor: JamiTheme.mosaicButtonNormalColor
                onButtonTextWidthChanged: {
                    if (buttonTextWidth > JamiTheme.mosaicButtonTextPreferredWidth) {
                        if (mosaicButton.Layout.preferredWidth + buttonTextWidth
                                - JamiTheme.mosaicButtonTextPreferredWidth
                                > JamiTheme.mosaicButtonMaxWidth) {
                            mosaicButton.Layout.preferredWidth = JamiTheme.mosaicButtonMaxWidth
                            buttonTextEnableElide = true
                        } else
                            mosaicButton.Layout.preferredWidth += buttonTextWidth
                                    - JamiTheme.mosaicButtonTextPreferredWidth
                    }
                }

                onClicked: CallAdapter.showGridConferenceLayout()
            }

            Text {
                id: callTimerText

                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.preferredHeight: 48
                Layout.rightMargin: recordingRect.visible ? 0 : JamiTheme.preferredMarginSize

                font.pointSize: JamiTheme.textFontSize
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter

                text: timeText
                color: JamiTheme.whiteColor
                elide: Qt.ElideRight
            }

            Rectangle {
                id: recordingRect
                visible: root.isRecording || root.remoteRecording

                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.rightMargin: JamiTheme.preferredMarginSize

                height: 16
                width: 16

                radius: height / 2
                color: JamiTheme.recordIconColor

                SequentialAnimation on color {
                    loops: Animation.Infinite
                    running: true
                    ColorAnimation {
                        from: JamiTheme.recordIconColor
                        to: "transparent"
                        duration: JamiTheme.recordBlinkDuration
                    }
                    ColorAnimation {
                        from: "transparent"
                        to: JamiTheme.recordIconColor
                        duration: JamiTheme.recordBlinkDuration
                    }
                }
            }
        }
    }

    ParticipantCallInStatusView {
        id: participantCallInStatusView

        anchors.right: root.right
        anchors.rightMargin: 10
        anchors.bottom: __callActionBar.top
        anchors.bottomMargin: 20
    }

    Rectangle {
        id: alertMessage

        anchors.bottom: __callActionBar.top
        anchors.bottomMargin: 16
        anchors.horizontalCenter: __callActionBar.horizontalCenter
        width: alertMessageTxt.width + 16
        height: alertMessageTxt.contentHeight + 16
        radius: 5
        visible: root.muteAlertActive
        color: JamiTheme.darkGreyColorOpacity

        Text {
            id: alertMessageTxt
            text: root.muteAlertMessage
            anchors.centerIn: parent
            width: Math.min(root.width, contentWidth)
            color: JamiTheme.whiteColor
            font.pointSize: JamiTheme.textFontSize
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // Timer to decide when ParticipantOverlay fade out
        Timer {
            id: alertTimer
            interval: JamiTheme.overlayFadeDelay
            onTriggered: {
                root.muteAlertActive = false
            }
        }
    }

    CallActionBar {
        id: __callActionBar

        anchors {
            bottom: parent.bottom
            bottomMargin: 26
        }

        width: parent.width
        height: 55
    }

    Behavior on opacity {
        NumberAnimation {
            duration: JamiTheme.overlayFadeDuration
        }
    }
}
