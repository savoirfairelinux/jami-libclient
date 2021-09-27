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
    // this offscreen caching is pretty huge
    // displayMarginEnd may be removed
    displayMarginBeginning: 4096
    displayMarginEnd: 4096
    maximumFlickVelocity: 2048
    verticalLayoutDirection: ListView.BottomToTop
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    currentIndex: -1

    ScrollBar.vertical: ScrollBar {}

    model: MessagesAdapter.messageListModel

    delegate: MessageDelegate {
        // sequencing/timestamps (2-sided style)
        function computeTimestampVisibility() {
            if (listView === undefined)
                return
            var nItem = listView.itemAtIndex(index - 1)
            if (nItem && index !== listView.count - 1) {
                showTime = (nItem.timestamp - timestamp) > 60 &&
                        nItem.formattedTime !== formattedTime
            } else {
                showTime = true
                var pItem = listView.itemAtIndex(index + 1)
                if (pItem) {
                    pItem.showTime = (timestamp - pItem.timestamp) > 60 &&
                            pItem.formattedTime !== formattedTime
                }
            }
        }

        function computeSequencing() {
            if (listView === undefined)
                return
            var cItem = {
                'author': author,
                'isGenerated': isGenerated,
                'showTime': showTime
            }
            var pItem = listView.itemAtIndex(index + 1)
            var nItem = listView.itemAtIndex(index - 1)

            let isSeq = (item0, item1) =>
                item0.author === item1.author &&
                !(item0.isGenerated || item1.isGenerated) &&
                !item0.showTime

            let setSeq = function (newSeq, item) {
                if (item === undefined)
                    seq = isGenerated ? MsgSeq.single : newSeq
                else
                    item.seq = item.isGenerated ? MsgSeq.single : newSeq
            }

            let rAdjustSeq = function (item) {
                if (item.seq === MsgSeq.last)
                    item.seq = MsgSeq.middle
                else if (item.seq === MsgSeq.single)
                    setSeq(MsgSeq.first, item)
            }

            let adjustSeq = function (item) {
                if (item.seq === MsgSeq.first)
                    item.seq = MsgSeq.middle
                else if (item.seq === MsgSeq.single)
                    setSeq(MsgSeq.last, item)
            }

            if (pItem && !nItem) {
                if (!isSeq(pItem, cItem)) {
                    seq = MsgSeq.single
                } else {
                    seq = MsgSeq.last
                    rAdjustSeq(pItem)
                }
            } else if (nItem && !pItem) {
                if (!isSeq(cItem, nItem)) {
                    seq = MsgSeq.single
                } else {
                    setSeq(MsgSeq.first)
                    adjustSeq(nItem)
                }
            } else if (!nItem && !pItem) {
                seq = MsgSeq.single
            } else {
                if (isSeq(pItem, nItem)) {
                    if (isSeq(pItem, cItem)) {
                        seq = MsgSeq.middle
                    } else {
                        seq = MsgSeq.single

                        if (pItem.seq === MsgSeq.first)
                            pItem.seq = MsgSeq.single
                        else if (item.seq === MsgSeq.middle)
                            pItem.seq = MsgSeq.last

                        if (nItem.seq === MsgSeq.last)
                            nItem.seq = MsgSeq.single
                        else if (nItem.seq === MsgSeq.middle)
                            nItem.seq = MsgSeq.first
                    }
                } else {
                    if (!isSeq(pItem, cItem)) {
                        seq = MsgSeq.first
                        adjustSeq(pItem)
                    } else {
                        seq = MsgSeq.last
                        rAdjustSeq(nItem)
                    }
                }
            }

            if (seq === MsgSeq.last) {
                showTime = true
            }
        }

        Component.onCompleted: {
            if (index) {
                computeTimestampVisibility()
                computeSequencing()
            } else {
                Qt.callLater(computeTimestampVisibility)
                Qt.callLater(computeSequencing)
            }
        }
    }

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
