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

import QtQuick
import QtQuick.Layouts

import net.jami.Constants 1.1

RowLayout {
    id: root

    Rectangle {
        id: descriptionTextRect

        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
        Layout.preferredHeight: descriptionText.contentHeight + 10
        Layout.preferredWidth: descriptionText.contentWidth + 10
        Layout.leftMargin: 10

        color: JamiTheme.transparentColor

        Text {
            id: descriptionText

            anchors.centerIn: parent

            text: description
            font.pointSize: JamiTheme.textFontSize
            font.weight: Font.Bold
            color: JamiTheme.textColor
        }
    }

    Rectangle {
        id: shortcutTextRect

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        Layout.preferredHeight: shortcutText.contentHeight + 10
        Layout.preferredWidth: shortcutText.contentWidth + 10
        Layout.rightMargin: 10

        color: JamiTheme.backgroundColor
        radius: JamiTheme.primaryRadius

        Text {
            id: shortcutText

            anchors.centerIn: parent

            text: shortcut
            font.pointSize: JamiTheme.textFontSize + 3
            font.weight: Font.DemiBold
            color: JamiTheme.textColor
        }
    }
}
