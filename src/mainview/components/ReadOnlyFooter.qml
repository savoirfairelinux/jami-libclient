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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Control {
    padding: 12

    background: Rectangle {
        anchors.fill: parent
        color: JamiTheme.primaryBackgroundColor

        Rectangle {
            anchors.top: parent.top
            height: JamiTheme.chatViewHairLineSize
            width: parent.width
            color: JamiTheme.tabbarBorderColor
        }
    }

    contentItem: ColumnLayout {
        spacing: 12
        Text {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true

            text: JamiStrings.contactLeft
            font.pointSize: JamiTheme.textFontSize + 2
            color: JamiTheme.textColor
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
            spacing: 12

            MaterialButton {
                text: JamiStrings.removeContact
                font.pointSize: JamiTheme.textFontSize + 2
                onClicked: MessagesAdapter.removeContact(
                               LRCInstance.selectedConvUid)
            }

            MaterialButton {
                text: JamiStrings.newConversation
                font.pointSize: JamiTheme.textFontSize + 2
                onClicked: ConversationsAdapter.restartConversation(
                               LRCInstance.selectedConvUid)
            }
        }
    }
}
