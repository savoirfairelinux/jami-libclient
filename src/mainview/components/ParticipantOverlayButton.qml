/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

import QtQuick

import net.jami.Constants 1.1

import "../../commoncomponents"

PushButton {
    id: root

    property alias toolTipText: toolTip.text

    normalColor: JamiTheme.buttonConference
    hoveredColor: JamiTheme.buttonConferenceHovered
    pressedColor: JamiTheme.buttonConferencePressed

    imageColor: JamiTheme.whiteColor

    Rectangle {
        id: toolTipRect
        height: 16
        width: toolTip.width + 8
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.bottom
            topMargin: isBarLayout? 6 : 2
        }
        color : isBarLayout? JamiTheme.darkGreyColorOpacity
                           : "transparent"
        visible: root.hovered && !isSmall
        radius: 2

        Text {
            id: toolTip
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            color: JamiTheme.whiteColor
            font.pointSize: JamiTheme.tinyFontSize
        }
    }
}
