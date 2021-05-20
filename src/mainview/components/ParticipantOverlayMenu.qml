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
import QtQuick.Shapes 1.14

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

// Overlay menu for conference moderation
Control {
    id: root

    property string uri: ""
    property string bestName: ""
    property bool isLocalMuted: true
    property bool showSetModerator: false
    property bool showUnsetModerator: false
    property bool showModeratorMute: false
    property bool showModeratorUnmute: false
    property bool showMaximize: false
    property bool showMinimize: false
    property bool showHangup: false

    property int buttonPreferredSize: 24
    property int iconButtonPreferredSize: 16

    property int visibleButtons: toggleModerator.visible
                                 + toggleMute.visible
                                 + maximizeParticipant.visible
                                 + minimizeParticipant.visible
                                 + hangupParticipant.visible

    property int buttonsSize: visibleButtons * 24 + 8 * 2

    property int shapeWidth: bestNameLabel.contentWidth + (visibleButtons > 0
                                                           ? buttonsSize : 0) + 32
    property int shapeHeight: 30
    property int shapeRadius: 8
    property string pathShape: "M0,0 h%1 v%2 q0,%3 -%3,%3 h-%4 z"
    .arg(shapeWidth).arg(shapeHeight-shapeRadius).arg(shapeRadius).
    arg(shapeWidth-shapeRadius)

    property bool isBarLayout: parent.width > 220
    property bool isOverlayRect: buttonsSize + 32 > parent.width

    property int labelMaxWidth: isBarLayout? Math.max(parent.width - buttonsSize, 80)
                                           : visibleButtons > 0? buttonsSize
                                                               : parent.width - 16

    property int isSmall: !isBarLayout && (height < 100 || width < 160)

    width: isBarLayout? bestNameLabel.contentWidth + buttonsSize + 32
                      : (isOverlayRect? buttonsSize + 32 : parent.width)
    height: isBarLayout? shapeHeight : (isOverlayRect? 80 : parent.height)

    anchors.top: isBarLayout? parent.top : undefined
    anchors.left: isBarLayout? parent.left : undefined
    anchors.centerIn: isBarLayout? undefined : parent

    background: Rectangle {
        color: isBarLayout? "transparent" : JamiTheme.darkGreyColorOpacity
        radius: (isBarLayout || !isOverlayRect)? 0 : 10
    }

    Item {
        anchors.fill: parent

        Shape {
            id: myShape
            visible: isBarLayout
            ShapePath {
                id: backgroundShape
                strokeColor: "transparent"
                fillColor: JamiTheme.darkGreyColorOpacity
                capStyle: ShapePath.RoundCap
                PathSvg { path: pathShape }
            }
        }

        Text {
            id: bestNameLabel
            anchors {
                left: isBarLayout? parent.left : undefined
                leftMargin: isBarLayout? 8 : 0
                bottom: isBarLayout? parent.bottom : undefined
                bottomMargin: isBarLayout? 8 : 0
                horizontalCenter: isBarLayout? undefined : parent.horizontalCenter
                verticalCenter: parent.verticalCenter
                verticalCenterOffset:
                    (isBarLayout || visibleButtons === 0)? 0 : (isSmall? -12 : -16)
            }
            TextMetrics {
                id: participantMetricsColumn
                text: bestName
                elide: Text.ElideRight
                elideWidth: labelMaxWidth
            }

            text: participantMetricsColumn.elidedText
            color: JamiTheme.whiteColor
            font.pointSize: JamiTheme.participantFontSize
            horizontalAlignment: isBarLayout? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Rectangle {
            color: "transparent"
            width: buttonsSize
            height: shapeHeight
            anchors {
                right: isBarLayout? parent.right : undefined
                rightMargin: isBarLayout? 8 : 0
                horizontalCenter: isBarLayout? undefined : parent.horizontalCenter
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: isBarLayout? 0 : (isSmall? 12 : 16)
            }

            RowLayout {
                id: rowLayoutButtons
                anchors.centerIn: parent
                anchors.fill: parent

                PushButton {
                    id: toggleModerator

                    visible: (showSetModerator || showUnsetModerator)
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: iconButtonPreferredSize
                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/moderator.svg"
                    imageColor: JamiTheme.whiteColor

                    onClicked: CallAdapter.setModerator(uri, showSetModerator)
                    onHoveredChanged:
                        toggleModeratorToolTip.visible = hovered && !isSmall

                    Rectangle {
                        id: toggleModeratorToolTip
                        height: 16
                        width: toggleModeratorToolTipText.width + 8
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.bottom
                            topMargin: isBarLayout? 6 : 2
                        }
                        color : isBarLayout? JamiTheme.darkGreyColorOpacity
                                           : "transparent"
                        visible: false
                        radius: 2

                        Text {
                            id: toggleModeratorToolTipText
                            anchors.centerIn: parent
                            text: showSetModerator? JamiStrings.setModerator
                                                  : JamiStrings.unsetModerator
                            horizontalAlignment: Text.AlignHCenter
                            color: JamiTheme.whiteColor
                            font.pointSize: JamiTheme.tinyFontSize
                        }
                    }
                }

                PushButton {
                    id: toggleMute

                    visible: showModeratorMute || showModeratorUnmute
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    Layout.alignment: Qt.AlignVCenter
                    preferredSize: iconButtonPreferredSize

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: showModeratorMute? "qrc:/images/icons/mic-24px.svg"
                                             : "qrc:/images/icons/mic_off-24px.svg"
                    imageColor: JamiTheme.whiteColor

                    onClicked: CallAdapter.muteParticipant(uri, showModeratorMute)
                    onHoveredChanged:
                        toggleMuteToolTip.visible = hovered && !isSmall

                    Rectangle {
                        id: toggleMuteToolTip
                        height: localMutedText.visible? 28 : 16
                        width: localMutedText.visible? localMutedText.width + 8
                                                     : toggleMuteToolTipText.width + 8
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.bottom
                            topMargin: isBarLayout? 6 : 2
                        }
                        color : isBarLayout? JamiTheme.darkGreyColorOpacity
                                           : "transparent"
                        visible: false
                        radius: 2

                        Text {
                            id: toggleMuteToolTipText
                            text: (showModeratorMute? JamiStrings.muteParticipant
                                                    : JamiStrings.unmuteParticipant)
                            horizontalAlignment: Text.AlignHCenter
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: parent.top
                            }

                            color: JamiTheme.whiteColor
                            font.pointSize: JamiTheme.tinyFontSize
                        }

                        Text {
                            id: localMutedText
                            visible: isLocalMuted
                            text: "(" + JamiStrings.localMuted + ")"
                            horizontalAlignment: Text.AlignHCenter
                            anchors {
                                top: toggleMuteToolTipText.bottom
                                horizontalCenter: parent.horizontalCenter
                            }
                            color: JamiTheme.whiteColor
                            font.pointSize: JamiTheme.tinyFontSize
                        }
                    }
                }

                PushButton {
                    id: maximizeParticipant

                    visible: showMaximize
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: iconButtonPreferredSize

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/open_in_full-24px.svg"
                    imageColor: JamiTheme.whiteColor

                    onClicked: CallAdapter.maximizeParticipant(uri)
                    onHoveredChanged:
                        maximizeParticipantToolTip.visible = hovered && !isSmall

                    Rectangle {
                        id: maximizeParticipantToolTip
                        height: 16
                        width: maximizeParticipantToolTipText.width + 8
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.bottom
                            topMargin: isBarLayout? 6 : 2
                        }
                        color : isBarLayout? JamiTheme.darkGreyColorOpacity
                                           : "transparent"
                        visible: false
                        radius: 2

                        Text {
                            id: maximizeParticipantToolTipText
                            text: JamiStrings.maximizeParticipant
                            horizontalAlignment: Text.AlignHCenter
                            anchors.centerIn: parent
                            color: JamiTheme.whiteColor
                            font.pointSize: JamiTheme.tinyFontSize
                        }
                    }
                }

                PushButton {
                    id: minimizeParticipant

                    visible: showMinimize
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: iconButtonPreferredSize

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/close_fullscreen-24px.svg"
                    imageColor: JamiTheme.whiteColor
                    onClicked: CallAdapter.minimizeParticipant(uri)
                    onHoveredChanged:
                        minimizeParticipantToolTip.visible = hovered && !isSmall

                    Rectangle {
                        id: minimizeParticipantToolTip
                        height: 16
                        width: minimizeParticipantToolTipText.width + 8
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.bottom
                            topMargin: isBarLayout? 6 : 2
                        }
                        color : isBarLayout? JamiTheme.darkGreyColorOpacity
                                           : "transparent"
                        visible: false
                        radius: 2

                        Text {
                            id: minimizeParticipantToolTipText
                            text: JamiStrings.minimizeParticipant
                            horizontalAlignment: Text.AlignHCenter
                            anchors.centerIn: parent
                            color: JamiTheme.whiteColor
                            font.pointSize: JamiTheme.tinyFontSize
                        }
                    }
                }

                PushButton {
                    id: hangupParticipant

                    visible: showHangup
                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize
                    preferredSize: iconButtonPreferredSize

                    normalColor: JamiTheme.buttonConference
                    hoveredColor: JamiTheme.buttonConferenceHovered
                    pressedColor: JamiTheme.buttonConferencePressed

                    source: "qrc:/images/icons/ic_hangup_participant-24px.svg"
                    imageColor: JamiTheme.whiteColor
                    onClicked: CallAdapter.hangupParticipant(uri)
                    onHoveredChanged:
                        hangupParticipantToolTip.visible = hovered && !isSmall

                    Rectangle {
                        id: hangupParticipantToolTip
                        height: 16
                        width: hangupParticipantToolTipText.width + 8
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: parent.bottom
                            topMargin: isBarLayout? 6 : 2
                        }
                        color : isBarLayout? JamiTheme.darkGreyColorOpacity
                                           : "transparent"
                        visible: false
                        radius: 2

                        Text {
                            id: hangupParticipantToolTipText
                            text: JamiStrings.hangupParticipant
                            horizontalAlignment: Text.AlignHCenter
                            anchors.centerIn: parent
                            color: JamiTheme.whiteColor
                            font.pointSize: JamiTheme.tinyFontSize
                        }
                    }
                }
            }
        }
    }
}
