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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14
import QtQml 2.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Item {
    id: root

    property string timeText: "00:00"
    property string remoteRecordingLabel: ""

    property string bestName: ""

    property alias recordingVisible: recordingRect.visible
    property alias callActionBar: __callActionBar

    property bool frozen: callActionBar.overflowOpen ||
                          callActionBar.hovered ||
                          callActionBar.subMenuOpen ||
                          participantCallInStatusView.visible

    opacity: 0

    // (un)subscribe to an app-wide mouse move event trap filtered
    // for the overlay's geometry
    onVisibleChanged: visible ?
                          CallOverlayModel.registerFilter(appWindow, this) :
                          CallOverlayModel.unregisterFilter(appWindow, this)

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
            if (root.opacity === 0 && !callViewContextMenu.peerIsRecording)
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
                    ColorAnimation {
                        from: "red"; to: "transparent";
                        duration: JamiTheme.recordBlinkDuration
                    }
                    ColorAnimation {
                        from: "transparent"; to: "red";
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

    CallActionBar {
        id: __callActionBar

        anchors {
            bottom: parent.bottom
            bottomMargin: 26
        }

        width: parent.width
        height: 55
    }

    Behavior on opacity { NumberAnimation { duration: JamiTheme.overlayFadeDuration }}
}
