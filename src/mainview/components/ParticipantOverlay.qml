/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property int buttonPreferredSize: 12
    property var uri: ""
    property var active: true
    property var isLocal: true
    property var showEndCall: true
    property var injectedContextMenu: null

    function setParticipantName(name) {
        participantName.text = name
    }

    // TODO: try to use AvatarImage as well
    function setAvatar(avatar) {
        if (avatar === "") {
            opacity = 0
            contactImage.source = ""
        } else {
            opacity = 1
            contactImage.source = "data:image/png;base64," + avatar
        }
    }

    function setMenuVisible(isVisible) {
        optionsButton.visible = isVisible
    }

    function setEndCallVisible(isVisible) {
        showEndCall = isVisible
    }

    border.width: 1
    opacity: 0
    color: "transparent"
    z: 1

    MouseArea {
        id: mouseAreaHover
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
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
                        var size = ((contactImage.width <= contactImage.height)? contactImage.width:contactImage.height)
                        return size /2
                    }
                }
            }
        }

        RowLayout {
            id: bottomLabel

            height: 24
            width: parent.width
            anchors.bottom: parent.bottom

            Rectangle {
                color: "black"
                opacity: 0.8
                height: parent.height
                width: parent.width
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                Text {
                    id: participantName
                    anchors.fill: parent
                    leftPadding: 8.0

                    TextMetrics {
                        id: participantMetrics
                        elide: Text.ElideRight
                        elideWidth: bottomLabel.width - 8
                    }

                    text: participantMetrics.elidedText

                    color: "white"
                    font.pointSize: JamiTheme.textFontSize
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Button {
                    id: optionsButton

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    background: Rectangle {
                        color: "transparent"
                    }


                    icon.color: "white"
                    icon.height: buttonPreferredSize
                    icon.width: buttonPreferredSize
                    icon.source: "qrc:/images/icons/more_vert-24px.svg"

                    onClicked: {
                        if (!injectedContextMenu) {
                            console.log("Participant's overlay don't have any injected context menu")
                            return
                        }
                        var mousePos = mapToItem(videoCallPageRect, parent.x, parent.y)
                        var layout = CallAdapter.getCurrentLayoutType()
                        var showMaximized = layout !== 2
                        var showMinimized = !(layout === 0 || (layout === 1 && !active))
                        var isModerator = CallAdapter.isModerator(uri)
                        var isHost = CallAdapter.isCurrentHost()
                        var participantIsHost = CallAdapter.participantIsHost(uri)
                        var isMuted = CallAdapter.isMuted(uri)
                        injectedContextMenu.showHangup = !root.isLocal && showEndCall
                        injectedContextMenu.showMaximize = showMaximized
                        injectedContextMenu.showMinimize = showMinimized
                        injectedContextMenu.uri = uri
                        injectedContextMenu.active = active
                        injectedContextMenu.x = mousePos.x
                        injectedContextMenu.y = mousePos.y - injectedContextMenu.height
                        injectedContextMenu.showSetModerator = (isHost && !participantIsHost && !isModerator)
                        injectedContextMenu.showUnsetModerator = (isHost && !participantIsHost && isModerator)
                        injectedContextMenu.showMute = !isMuted
                        injectedContextMenu.showUnmute = isMuted && root.isLocal
                        injectedContextMenu.openMenu()
                    }
                }
            }
        }

        onClicked: {
            CallAdapter.maximizeParticipant(uri, active)
        }

        onEntered: {
            if (contactImage.status === Image.Null)
                root.state = "entered"
        }

        onExited: {
            if (contactImage.status === Image.Null)
                root.state = "exited"
        }
    }

    states: [
        State {
            name: "entered"
            PropertyChanges {
                target: root
                opacity: 1
            }
        },
        State {
            name: "exited"
            PropertyChanges {
                target: root
                opacity: 0
            }
        }
    ]

    transitions: Transition {
        PropertyAnimation {
            target: root
            property: "opacity"
            duration: 500
        }
    }
}
