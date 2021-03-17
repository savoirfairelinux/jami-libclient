/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

// Overlay menu for conference moderation
Rectangle {
    id: root

    property bool hasMinimumSize: true
    property int buttonPreferredSize: 30
    property int minimumWidth: Math.max(114, visibleButtons * 37 + 21 * 2)
    property int minimumHeight: 114
    property int visibleButtons: toggleModerator.visible
                                 + toggleMute.visible
                                 + maximizeParticipant.visible
                                 + minimizeParticipant.visible
                                 + hangupParticipant.visible

    property string uri: ""
    property string bestName: ""
    property bool isLocalMuted: false
    property bool showSetModerator: false
    property bool showUnsetModerator: false
    property bool showModeratorMute: false
    property bool showModeratorUnmute: false
    property bool showMaximize: false
    property bool showMinimize: false
    property bool showHangup: false

    signal mouseAreaExited

    // values taken from sketch
    width: hasMinimumSize? parent.width : minimumWidth
    height: hasMinimumSize? parent.height: minimumHeight

    color: hasMinimumSize? "transparent" : JamiTheme.darkGreyColorOpacity
    radius: 10

    MouseArea {
        id: mouseAreaHover

        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton

        onExited: mouseAreaExited()

        ColumnLayout {
            id: layout
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Text {
                id: participantName

                TextMetrics {
                    id: participantMetrics
                    text: bestName
                    elide: Text.ElideRight
                    elideWidth: root.width - JamiTheme.preferredMarginSize * 2
                }

                text: participantMetrics.elidedText
                color: JamiTheme.whiteColor
                font.pointSize: JamiTheme.participantFontSize
                Layout.alignment: Qt.AlignCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            RowLayout {
                id: rowLayoutButtons

                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.fillWidth: true
                spacing: 7

                PushButton {
                    id: toggleModerator

                    visible: (showSetModerator || showUnsetModerator)
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: 16
                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/moderator.svg"
                    imageColor: hovered? JamiTheme.darkGreyColor
                                       : JamiTheme.whiteColor

                    onClicked: CallAdapter.setModerator(uri, showSetModerator)
                    onHoveredChanged: toggleModeratorToolTip.visible = hovered

                    Text {
                        id: toggleModeratorToolTip

                        visible: false
                        width: parent.width
                        text: showSetModerator? JamiStrings.setModerator
                                              : JamiStrings.unsetModerator
                        horizontalAlignment: Text.AlignHCenter
                        anchors.top: parent.bottom
                        anchors.topMargin: 6
                        color: JamiTheme.whiteColor
                        font.pointSize: JamiTheme.tinyFontSize
                    }
                }

                PushButton {
                    id: toggleMute

                    visible: showModeratorMute || showModeratorUnmute
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: 16

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: showModeratorMute? "qrc:/images/icons/mic-24px.svg"
                                             : "qrc:/images/icons/mic_off-24px.svg"
                    imageColor: hovered? JamiTheme.darkGreyColor
                                       : JamiTheme.whiteColor

                    onClicked: CallAdapter.muteParticipant(uri, showModeratorMute)
                    onHoveredChanged: {
                        toggleParticipantToolTip.visible = hovered
                        localMutedText.visible = hovered && isLocalMuted
                    }

                    Text {
                        id: toggleParticipantToolTip

                        visible: false
                        width: parent.width
                        text: showModeratorMute? JamiStrings.muteParticipant
                                               : JamiStrings.unmuteParticipant
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignTop

                        anchors.top: parent.bottom
                        anchors.topMargin: 6
                        color: JamiTheme.whiteColor
                        font.pointSize: JamiTheme.tinyFontSize
                    }

                    Text {
                        id: localMutedText

                        visible: false
                        width: parent.width
                        text: "(" + JamiStrings.localMuted + ")"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignTop

                        anchors.top: parent.bottom
                        anchors.topMargin: 16
                        color: JamiTheme.whiteColor
                        font.pointSize: JamiTheme.tinyFontSize
                    }

                }

                PushButton {
                    id: maximizeParticipant

                    visible: showMaximize
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: 16

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/open_in_full-24px.svg"
                    imageColor: hovered? JamiTheme.darkGreyColor
                                       : JamiTheme.whiteColor

                    onClicked: CallAdapter.maximizeParticipant(uri)
                    onHoveredChanged: maximizeParticipantToolTip.visible = hovered

                    Text {
                        id: maximizeParticipantToolTip

                        visible: false
                        width: parent.width
                        text: JamiStrings.maximizeParticipant
                        horizontalAlignment: Text.AlignHCenter
                        anchors.top: parent.bottom
                        anchors.topMargin: 6
                        color: JamiTheme.whiteColor
                        font.pointSize: JamiTheme.tinyFontSize
                    }
                }

                PushButton {
                    id: minimizeParticipant

                    visible: showMinimize
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: 16

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/close_fullscreen-24px.svg"
                    imageColor: hovered? JamiTheme.darkGreyColor
                                       : JamiTheme.whiteColor
                    onClicked: CallAdapter.minimizeParticipant(uri)
                    onHoveredChanged: minimizeParticipantToolTip.visible = hovered

                    Text {
                        id: minimizeParticipantToolTip

                        visible: false
                        width: parent.width
                        text: JamiStrings.minimizeParticipant
                        horizontalAlignment: Text.AlignHCenter
                        anchors.top: parent.bottom
                        anchors.topMargin: 6
                        color: JamiTheme.whiteColor
                        font.pointSize: JamiTheme.tinyFontSize
                    }
                }

                PushButton {
                    id: hangupParticipant

                    visible: showHangup
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: 16

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/ic_block_24px.svg"
                    imageColor: hovered? JamiTheme.darkGreyColor
                                       : JamiTheme.whiteColor
                    onClicked: CallAdapter.hangupParticipant(uri)
                    onHoveredChanged: hangupParticipantToolTip.visible = hovered

                    Text {
                        id: hangupParticipantToolTip

                        visible: false
                        width: parent.width
                        text: JamiStrings.hangupParticipant
                        horizontalAlignment: Text.AlignHCenter
                        anchors.top: parent.bottom
                        anchors.topMargin: 6
                        color: JamiTheme.whiteColor
                        font.pointSize: JamiTheme.tinyFontSize
                    }
                }
            }
        }
    }
}
