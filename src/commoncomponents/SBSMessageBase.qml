/*
 * Copyright (C) 2021 by Savoir-faire Linux
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
import QtGraphicalEffects 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

Control {
    id: root

    property alias avatarBlockWidth: avatarBlock.width
    property alias innerContent: innerContent
    property alias bubble: bubble
    property real extraHeight: 0

    // these MUST be set but we won't use the 'required' keyword yet
    property bool isOutgoing
    property bool showTime
    property int seq
    property string author
    property string transferId
    property string transferName
    property string formattedTime
    property string location
    property string hoveredLink
    property var readers: []

    readonly property real senderMargin: 64
    readonly property real avatarSize: 32
    readonly property real msgRadius: 18
    readonly property real hPadding: 12

    width: ListView.view ? ListView.view.width : 0
    height: mainColumnLayout.implicitHeight

    rightPadding: hPadding
    leftPadding: hPadding

    contentItem: ColumnLayout {
        id: mainColumnLayout

        anchors.centerIn: parent

        width: parent.width

        spacing: 2

        RowLayout {
            Layout.preferredHeight: innerContent.height + root.extraHeight
            Layout.topMargin: (seq === MsgSeq.first || seq === MsgSeq.single) ? 6 : 0
            spacing: 0
            Item {
                id: avatarBlock
                Layout.preferredWidth: isOutgoing ? 0 : avatar.width + hPadding
                Layout.preferredHeight: isOutgoing ? 0 : bubble.height
                Avatar {
                    id: avatar
                    visible: !isOutgoing && (seq === MsgSeq.last || seq === MsgSeq.single)
                    anchors.bottom: parent.bottom
                    width: avatarSize
                    height: avatarSize
                    imageId: author
                    showPresenceIndicator: false
                    mode: Avatar.Mode.Contact
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Column {
                    id: innerContent
                    width: parent.width
                    // place actual content here
                }
                MessageBubble {
                    id: bubble
                    z:-1
                    out: isOutgoing
                    type: seq
                    color: isOutgoing ?
                               JamiTheme.messageOutBgColor :
                               JamiTheme.messageInBgColor
                    radius: msgRadius
                    anchors.right: isOutgoing ? parent.right : undefined
                    width: innerContent.childrenRect.width
                    height: innerContent.childrenRect.height + (visible ? root.extraHeight : 0)
                }
            }
        }
        ListView {
            id: infoCell

            Layout.fillWidth: true
            orientation: ListView.Horizontal
            Layout.preferredHeight: {
                if (showTime || seq === MsgSeq.last)
                    return childrenRect.height
                else if (reads.visible)
                    return JamiTheme.avatarReadReceiptSize
                return 0
            }

            Label {
                text: formattedTime
                color: JamiTheme.timestampColor
                visible: showTime || seq === MsgSeq.last
                height: visible * implicitHeight
                font.pointSize: 9

                anchors.right: !isOutgoing ? undefined : reads.left
                anchors.rightMargin: 8
                anchors.left: isOutgoing ? undefined : parent.left
                anchors.leftMargin: avatarBlockWidth + 6
            }
            ReadStatus {
                id: reads
                visible: root.readers.length !== 0 && CurrentAccount.sendReadReceipt
                width: {
                    if (root.readers.length === 0)
                        return 0
                    var nbAvatars = root.readers.length
                    var margin = JamiTheme.avatarReadReceiptSize / 3
                    return nbAvatars * JamiTheme.avatarReadReceiptSize - (nbAvatars - 1) * margin
                }
                anchors.right: parent.right
                readers: root.readers
            }
        }
    }

    SBSContextMenu {
        id: ctxMenu

        location: root.location
        transferId: root.transferId
        transferName: root.transferName
    }

    MouseArea {
        id: itemMouseArea
        anchors.fill: parent
        z: -1
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: function (mouse) {

            if (mouse.button === Qt.RightButton && transferId !== "") {
                // Context Menu for Transfers
                ctxMenu.x = mouse.x
                ctxMenu.y = mouse.y
                ctxMenu.openMenu()
            } else if (root.hoveredLink)
                MessagesAdapter.openUrl(root.hoveredLink)
        }
    }
}
