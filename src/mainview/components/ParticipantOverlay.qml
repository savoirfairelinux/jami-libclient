/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtQuick.Layouts 1.14
import QtQuick.Shapes 1.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.14

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    // svg path for the participant indicators background shape
    property int shapeWidth: indicatorsRowLayout.width + 8
    property int shapeHeight: 16
    property int shapeRadius: 6
    property string pathShape: "M0,0 h%1 q%2,0 %2,%2 v%3 h-%4 z"
    .arg(shapeWidth-shapeRadius).arg(shapeRadius).arg(shapeHeight-shapeRadius).
    arg(shapeWidth)

    property string uri: overlayMenu.uri
    property bool participantIsHost: false
    property bool participantIsModerator: false
    property bool participantIsMuted: false
    property bool participantIsLocalMuted: false
    property bool participantIsModeratorMuted: false

    property bool participantMenuActive: false

    // TODO: try to use AvatarImage as well
    function setAvatar(avatar) {
        if (avatar === "") {
            contactImage.source = ""
        } else {
            contactImage.source = JamiQmlUtils.base64StringTitle + avatar
        }
    }

    function setMenu(newUri, bestName, isLocal, showMax, showMin) {

        overlayMenu.uri = newUri
        overlayMenu.bestName = bestName

        var isHost = CallAdapter.isCurrentHost()
        var isModerator = CallAdapter.isCurrentModerator()
        participantIsHost = CallAdapter.participantIsHost(overlayMenu.uri)
        participantIsModerator = CallAdapter.isModerator(overlayMenu.uri)
        overlayMenu.showSetModerator = isHost && !isLocal && !participantIsModerator
        overlayMenu.showUnsetModerator = isHost && !isLocal && participantIsModerator

        var muteState = CallAdapter.getMuteState(overlayMenu.uri)
        overlayMenu.isLocalMuted = muteState === CallAdapter.LOCAL_MUTED
                || muteState === CallAdapter.BOTH_MUTED
        var isModeratorMuted = muteState === CallAdapter.MODERATOR_MUTED
                || muteState === CallAdapter.BOTH_MUTED

        participantIsMuted = overlayMenu.isLocalMuted || isModeratorMuted

        overlayMenu.showModeratorMute = isModerator && !isModeratorMuted
        overlayMenu.showModeratorUnmute = isModerator && isModeratorMuted
        overlayMenu.showMaximize = isModerator && showMax
        overlayMenu.showMinimize = isModerator && showMin
        overlayMenu.showHangup = isModerator && !isLocal && !participantIsHost
    }

    color: "transparent"
    z: 1

    // Participant header with host, moderator and mute indicators
    Rectangle {
        id: participantIndicators
        width: indicatorsRowLayout.width
        height: shapeHeight
        visible: participantIsHost || participantIsModerator || participantIsMuted
        color: "transparent"
        anchors.bottom: parent.bottom

        Shape {
            id: backgroundShape
            ShapePath {
                id: backgroundShapePath
                strokeColor: "transparent"
                fillColor: JamiTheme.darkGreyColorOpacity
                capStyle: ShapePath.RoundCap
                PathSvg { path: pathShape }
            }
        }

        RowLayout {
            id: indicatorsRowLayout
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter

            ResponsiveImage {
                id: isHostIndicator

                visible: participantIsHost

                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6
                containerHeight: 12
                containerWidth: 12

                source: "qrc:/images/icons/star_outline-24px.svg"
                layer {
                    enabled: true
                    effect: ColorOverlay { color: JamiTheme.whiteColor }
                    mipmap: false
                    smooth: true
                }
            }

            ResponsiveImage {
                id: isModeratorIndicator

                visible: participantIsModerator

                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6
                containerHeight: 12
                containerWidth: 12

                source: "qrc:/images/icons/moderator.svg"
                layer {
                    enabled: true
                    effect: ColorOverlay { color: JamiTheme.whiteColor }
                    mipmap: false
                    smooth: true
                }
            }

            ResponsiveImage {
                id: isMutedIndicator

                visible: participantIsMuted
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6
                containerHeight: 12
                containerWidth: 12

                source: "qrc:/images/icons/mic_off-24px.svg"
                layer {
                    enabled: true
                    effect: ColorOverlay { color: JamiTheme.whiteColor }
                    mipmap: false
                    smooth: true
                }
            }
        }
    }

    // Participant background, mousearea, hover and buttons for moderation
    Rectangle {
        id: participantRect

        anchors.fill: parent
        opacity: 0
        color: "transparent"
        z: 1

        MouseArea {
            id: mouseAreaHover

            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: false
            acceptedButtons: Qt.LeftButton

            Image {
                id: contactImage

                anchors.centerIn: parent
                height:  Math.min(parent.width / 2, parent.height / 2)
                width:  Math.min(parent.width / 2, parent.height / 2)

                fillMode: Image.PreserveAspectFit
                source: ""
                asynchronous: true

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle{
                        width: contactImage.width
                        height: contactImage.height
                        radius: {
                            var size = ((contactImage.width <= contactImage.height)?
                                            contactImage.width : contactImage.height)
                            return size / 2
                        }
                    }
                }
                layer.mipmap: false
                layer.smooth: true
            }

            ParticipantOverlayMenu {
                id: overlayMenu
                visible: participantRect.opacity !== 0

                onMouseAreaExited: {
                    if (contactImage.status === Image.Null) {
                        root.z = 1
                        participantRect.state = "exited"
                    }
                }
                onMouseChanged: {
                    participantRect.state = "entered"
                    fadeOutTimer.restart()
                    participantMenuActive = true
                }
            }

            onClicked: {
                CallAdapter.maximizeParticipant(uri)
            }

            onEntered: {
                if (contactImage.status === Image.Null) {
                    root.z = 2
                    participantRect.state = "entered"
                }
            }

            onExited: {
                if (contactImage.status === Image.Null) {
                    root.z = 1
                    participantRect.state = "exited"
                }
            }

            onMouseXChanged: {
                // Hack: avoid listening mouseXChanged emitted when
                // ParticipantOverlayMenu is exited
                if (participantMenuActive) {
                    participantMenuActive = false
                } else {
                    participantRect.state = "entered"
                    fadeOutTimer.restart()
                }
            }
        }

        states: [
            State {
                name: "entered"
                PropertyChanges {
                    target: participantRect
                    opacity: 1
                }
            },
            State {
                name: "exited"
                PropertyChanges {
                    target: participantRect
                    opacity: 0
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation {
                target: participantRect
                property: "opacity"
                duration: 50
            }
        }
    }

    // Timer to decide when ParticipantOverlay fade out
    Timer {
        id: fadeOutTimer
        interval: 5000
        onTriggered: participantRect.state = "exited"
    }
}
