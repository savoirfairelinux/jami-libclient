/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Trevor Tabah <trevor.tabah@savoirfairelinux.com>
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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ListView {
    id: root

    // fade-in mechanism
    Component.onCompleted: fadeAnimation.start()
    Rectangle {
        id: overlay
        anchors.fill: parent
        color: JamiTheme.chatviewBgColor
        visible: opacity !== 0
        SequentialAnimation {
            id: fadeAnimation
            NumberAnimation {
                target: overlay; property: "opacity"
                to: 1; duration: 0
            }
            NumberAnimation {
                target: overlay; property: "opacity"
                to: 0; duration: 240
            }
        }
    }
    Connections {
        target: CurrentConversation
        function onIdChanged() { fadeAnimation.start() }
    }

    topMargin: 12
    bottomMargin: 6
    spacing: 2
    anchors.centerIn: parent
    height: parent.height
    width: parent.width
    displayMarginBeginning: 2048
    displayMarginEnd: 2048
    maximumFlickVelocity: 2048
    verticalLayoutDirection: ListView.BottomToTop
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    currentIndex: -1

    ScrollBar.vertical: ScrollBar {}

    model: MessagesAdapter.messageListModel

    delegate: MessageDelegate {}

    function getDistanceToBottom() {
        const scrollDiff = ScrollBar.vertical.position -
                         (1.0 - ScrollBar.vertical.size)
        return Math.abs(scrollDiff) * contentHeight
    }

    onAtYBeginningChanged: loadMoreMsgsIfNeeded()

    function loadMoreMsgsIfNeeded() {
        if (atYBeginning && !CurrentConversation.allMessagesLoaded)
            MessagesAdapter.loadMoreMessages()
    }

    Connections {
        target: MessagesAdapter

        function onNewInteraction() {
            if (root.getDistanceToBottom() < 80 &&
                    !root.atYEnd) {
                Qt.callLater(root.positionViewAtBeginning)
            }
        }

        function onMoreMessagesLoaded() {
            if (root.contentHeight < root.height) {
                root.loadMoreMsgsIfNeeded()
            }
        }
    }
}
