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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Item {
    id: root

    // svg path for the participant indicators background shape
    property int shapeWidth: indicatorsRowLayout.width + 8
    property int shapeHeight: 16
    property int shapeRadius: 6
    property string pathShape: "M0,0 h%1 q%2,0 %2,%2 v%3 h-%4 z"
        .arg(shapeWidth - shapeRadius)
        .arg(shapeRadius)
        .arg(shapeHeight - shapeRadius)
        .arg(shapeWidth)

    property string uri: overlayMenu.uri
    property bool participantIsActive: false
    property bool participantIsHost: false
    property bool participantIsModerator: false
    property bool participantIsMuted: false
    property bool participantIsModeratorMuted: false
    property bool participantHandIsRaised: false

    z: 1

    function setAvatar(show, uri, isLocal) {
        if (!show)
            avatar.active = false
        else {
            avatar.mode_ = isLocal ? Avatar.Mode.Account : Avatar.Mode.Contact
            avatar.imageId_ = isLocal ? LRCInstance.currentAccountId : uri
            avatar.active = true
        }
    }

    function setMenu(newUri, bestName, isLocal, isActive, showMax) {

        overlayMenu.uri = newUri
        overlayMenu.bestName = bestName

        var isHost = CallAdapter.isCurrentHost()
        var isModerator = CallAdapter.isCurrentModerator()
        participantIsHost = CallAdapter.participantIsHost(overlayMenu.uri)
        participantIsModerator = CallAdapter.isModerator(overlayMenu.uri)
        participantIsActive = isActive
        participantHandIsRaised = CallAdapter.isHandRaised(overlayMenu.uri)
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
        overlayMenu.showMinimize = isModerator && participantIsActive
        overlayMenu.showHangup = isModerator && !isLocal && !participantIsHost
        overlayMenu.showLowerHand = isModerator && participantHandIsRaised
    }

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

                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6

                containerHeight: 12
                containerWidth: 12

                visible: participantIsHost

                source: JamiResources.star_outline_24dp_svg
                color: JamiTheme.whiteColor
            }

            ResponsiveImage {
                id: isModeratorIndicator

                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6

                containerHeight: 12
                containerWidth: 12

                visible: participantIsModerator

                source: JamiResources.moderator_svg
                color: JamiTheme.whiteColor
            }

            ResponsiveImage {
                id: isMutedIndicator

                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6

                containerHeight: 12
                containerWidth: 12

                visible: participantIsMuted

                source: JamiResources.micro_off_black_24dp_svg
                color: JamiTheme.whiteColor
            }

            ResponsiveImage {
                id: isHandRaisedIndicator

                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 6

                containerHeight: 12
                containerWidth: 12

                visible: participantHandIsRaised

                source: JamiResources.hand_black_24dp_svg
                color: JamiTheme.whiteColor
            }
        }
    }

    Loader {
        id: avatar

        anchors.centerIn: parent

        active: false

        property real size_: Math.min(parent.width / 2, parent.height / 2)
        height:  size_
        width:  size_

        property int mode_
        property string imageId_

        sourceComponent: Component {
            Avatar {
                // round the avatar source size up to some nearest multiple
                readonly property real step: 96
                property real size: Math.floor((size_ + step - 1) / step) * step
                sourceSize: Qt.size(size, size)
                mode: mode_
                imageId: size_ ? imageId_ : ""
                showPresenceIndicator: false
            }
        }
    }

    // Participant background and buttons for moderation
    MouseArea {
        id: participantRect

        anchors.fill: parent
        opacity: 0
        z: 1

        propagateComposedEvents: true
        hoverEnabled: true
        onPositionChanged: {
            participantRect.opacity = 1
            fadeOutTimer.restart()
            // Here we could call: root.parent.positionChanged(mouse)
            // to relay the event to a main overlay mouse area, either
            // as a parent object or some property passed in. But, this
            // will still fail when hovering over menus, etc.
        }
        onExited: {
            root.z = 1
            participantRect.opacity = 0
        }
        onEntered: {
            root.z = 2
            participantRect.opacity = 1
        }

        // Timer to decide when ParticipantOverlay fade out
        Timer {
            id: fadeOutTimer
            interval: JamiTheme.overlayFadeDelay
            onTriggered: {
                if (overlayMenu.hovered)
                    return
                participantRect.opacity = 0
            }
        }

        ParticipantOverlayMenu { id: overlayMenu }

        Behavior on opacity { NumberAnimation { duration: JamiTheme.shortFadeDuration }}
    }
}
