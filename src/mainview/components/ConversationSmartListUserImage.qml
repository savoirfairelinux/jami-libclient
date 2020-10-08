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
import net.jami.Models 1.0
import "../../commoncomponents"

Image {
    id: userImage

    width: 40
    height: 40

    fillMode: Image.PreserveAspectFit
    source: "data:image/png;base64," + Picture
    mipmap: true

    PresenceIndicator {
        anchors.right: userImage.right
        anchors.bottom: userImage.bottom

        visible: Presence === undefined ? false : Presence
    }

    Rectangle {
        id: unreadMessageCountRect

        anchors.right: userImage.right
        anchors.rightMargin: -2
        anchors.top: userImage.top
        anchors.topMargin: -2

        width: 14
        height: 14

        visible: UnreadMessagesCount > 0

        Text {
            id: unreadMessageCounttext

            anchors.centerIn: unreadMessageCountRect

            text: UnreadMessagesCount > 9 ? "···" : UnreadMessagesCount
            color: "white"
            font.pointSize: JamiTheme.textFontSize
        }

        radius: 30
        color: JamiTheme.notificationRed
    }
}
