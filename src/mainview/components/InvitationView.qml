/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
import QtGraphicalEffects 1.14

import net.jami.Adapters 1.0
import net.jami.Constants 1.0
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property string invitationViewImageId
    property string invitationViewContactTitle
    property bool invitationViewNeedSyncing: false

    property real marginSize: 20
    property real textMarginSize: 50

    color: JamiTheme.primaryBackgroundColor

    Text {
        id: invitationViewSentRequestText

        anchors.top: root.top
        anchors.topMargin: visible ? marginSize : 0
        anchors.horizontalCenter: root.horizontalCenter

        width: infoColumnLayout.width - textMarginSize
        height: visible ? contentHeight : 0

        visible: !invitationViewNeedSyncing

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        font.pointSize: JamiTheme.textFontSize
        color: JamiTheme.textColor
        wrapMode: Text.Wrap

        text: JamiStrings.invitationViewSentRequest.arg(invitationViewContactTitle)
    }

    ColumnLayout {
        id: infoColumnLayout

        anchors.centerIn: root

        width: root.width

        Avatar {
            id: avatarImage

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: invitationViewSentRequestText.visible ? marginSize : 0
            Layout.preferredHeight: JamiTheme.invitationViewAvatarSize
            Layout.preferredWidth: JamiTheme.invitationViewAvatarSize

            showPresenceIndicator: false
            mode: Avatar.Mode.Contact
            imageId: invitationViewImageId
        }

        Text {
            id: invitationViewMiddlePhraseText

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: marginSize
            Layout.preferredWidth: infoColumnLayout.width - textMarginSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.weight: Font.DemiBold
            font.pointSize: JamiTheme.textFontSize + 3
            color: JamiTheme.textColor
            wrapMode: Text.Wrap

            text: root.invitationViewNeedSyncing ?
                      JamiStrings.invitationViewAcceptedConversation :
                      JamiStrings.invitationViewJoinConversation
        }

        Text {
            id: invitationViewWaitingForSyncText

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: marginSize
            Layout.preferredWidth: infoColumnLayout.width - textMarginSize
            Layout.preferredHeight: visible ? contentHeight : 0

            visible: invitationViewNeedSyncing

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.pointSize: JamiTheme.textFontSize
            color: JamiTheme.textColor
            wrapMode: Text.Wrap

            text: JamiStrings.invitationViewWaitingForSync.arg(invitationViewContactTitle)
        }

        RowLayout {
            id: buttonGroupRowLayout

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: marginSize

            spacing: JamiTheme.invitationViewButtonsSpacing

            visible: !invitationViewNeedSyncing

            PushButton {
                id: blockButton

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: JamiTheme.invitationViewButtonSize
                Layout.preferredWidth: JamiTheme.invitationViewButtonSize

                preferredSize: JamiTheme.invitationViewButtonIconSize
                radius: JamiTheme.invitationViewButtonRadius

                toolTipText: JamiStrings.blockContact

                source: "qrc:/images/icons/block_black-24dp.svg"
                imageColor: JamiTheme.primaryBackgroundColor

                normalColor: JamiTheme.blockOrangeTransparency
                pressedColor: JamiTheme.blockOrange
                hoveredColor: JamiTheme.blockOrange

                onClicked: MessagesAdapter.blockConversation()
            }

            PushButton {
                id: refuseButton

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: JamiTheme.invitationViewButtonSize
                Layout.preferredWidth: JamiTheme.invitationViewButtonSize

                preferredSize: JamiTheme.invitationViewButtonSize
                radius: JamiTheme.invitationViewButtonRadius

                toolTipText: JamiStrings.declineContactRequest

                source: "qrc:/images/icons/cross_black_24dp.svg"
                imageColor: JamiTheme.primaryBackgroundColor

                normalColor: JamiTheme.refuseRedTransparent
                pressedColor: JamiTheme.refuseRed
                hoveredColor: JamiTheme.refuseRed

                onClicked: MessagesAdapter.refuseInvitation()
            }

            PushButton {
                id: acceptButton

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: JamiTheme.invitationViewButtonSize
                Layout.preferredWidth: JamiTheme.invitationViewButtonSize

                preferredSize: JamiTheme.invitationViewButtonIconSize
                radius: JamiTheme.invitationViewButtonRadius

                toolTipText: JamiStrings.acceptContactRequest

                source: "qrc:/images/icons/check_black-24dp.svg"
                imageColor: JamiTheme.primaryBackgroundColor

                normalColor: JamiTheme.acceptGreenTransparency
                pressedColor: JamiTheme.acceptGreen
                hoveredColor: JamiTheme.acceptGreen

                onClicked: MessagesAdapter.acceptInvitation()
            }
        }
    }
}
