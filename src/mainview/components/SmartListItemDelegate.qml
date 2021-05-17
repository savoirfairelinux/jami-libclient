/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ItemDelegate {
    id: root

    width: ListView.view.width
    height: JamiTheme.smartListItemHeight

    function convUid() {
        return UID
    }

    Component.onCompleted: {
        if (ContactType === Profile.Type.TEMPORARY)
            root.ListView.view.model.updateContactAvatarUid(URI)
        avatar.updateImage(URI, PictureUid)
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        spacing: 10

        AvatarImage {
            id: avatar

            Connections {
                target: root.ListView.view.model
                function onDataChanged(index) {
                    var model = root.ListView.view.model
                    avatar.updateImage(URI === undefined ?
                                           model.data(index, ConversationList.URI):
                                           URI,
                                       PictureUid === undefined ?
                                           model.data(index, ConversationList.PictureUid):
                                           PictureUid)
                }
            }

            Layout.preferredWidth: JamiTheme.smartListAvatarSize
            Layout.preferredHeight: JamiTheme.smartListAvatarSize

            avatarMode: AvatarImage.AvatarMode.FromContactUri
            showPresenceIndicator: Presence === undefined ? false : Presence
            transitionDuration: 0
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            // best name
            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter
                elide: Text.ElideRight
                text: BestName === undefined ? "" : BestName
                font.pointSize: JamiTheme.smartlistItemFontSize
                font.weight: UnreadMessagesCount ? Font.Bold : Font.Normal
                color: JamiTheme.textColor
            }
            RowLayout {
                visible: ContactType !== Profile.Type.TEMPORARY
                         && LastInteractionDate !== undefined
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignTop
                // last Interaction date
                Text {
                    Layout.alignment: Qt.AlignVCenter
                    text: LastInteractionDate === undefined ? "" : LastInteractionDate
                    font.pointSize: JamiTheme.smartlistItemInfoFontSize
                    font.weight: UnreadMessagesCount ? Font.DemiBold : Font.Normal
                    color: JamiTheme.textColor
                }
                // last Interaction
                Text {
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    text: Draft ?
                              Draft :
                              (LastInteraction === undefined ? "" : LastInteraction)
                    font.pointSize: JamiTheme.smartlistItemInfoFontSize
                    font.weight: UnreadMessagesCount ? Font.Normal : Font.Light
                    font.hintingPreference: Font.PreferNoHinting
                    maximumLineCount: 1
                    color: JamiTheme.textColor
                    // deal with poor rendering of the pencil emoji on Windows
                    font.family: Qt.platform.os === "windows" && Draft ?
                                     "Segoe UI Emoji" :
                                     Qt.application.font.family
                    lineHeight: font.family === "Segoe UI Emoji" ? 1.25 : 1
                }
            }
        }

        ColumnLayout {
            visible: InCall || UnreadMessagesCount
            Layout.preferredWidth: childrenRect.width
            Layout.fillHeight: true
            spacing: 2
            // call status
            Text {
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignRight
                text: InCall ? UtilsAdapter.getCallStatusStr(CallState) : ""
                font.pointSize: JamiTheme.smartlistItemInfoFontSize
                font.weight: Font.Medium
                color: JamiTheme.textColor
            }
            // unread message count
            Item {
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                BadgeNotifier {
                    size: 20
                    count: UnreadMessagesCount
                    animate: index === 0
                }
            }
        }
    }

    background: Rectangle {
        color: {
            if (root.pressed)
                return Qt.darker(JamiTheme.selectedColor, 1.1)
            else if (root.hovered)
                return Qt.darker(JamiTheme.selectedColor, 1.05)
            else
                return "transparent"
        }
    }

    onClicked: ListView.view.model.select(index)
    onDoubleClicked: {
        ListView.view.model.select(index)
        if (LRCInstance.currentAccountType === Profile.Type.SIP)
            CallAdapter.placeAudioOnlyCall()
        else
            CallAdapter.placeCall()

        // TODO: factor this out (visible should be observing)
        communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
    }
    onPressAndHold: ListView.view.openContextMenuAt(pressX, pressY, root)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.ListView.view.openContextMenuAt(mouse.x, mouse.y, root)
    }
}
