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
import QtQuick.Controls

import net.jami.Constants 1.1

// Assumed to be attached to Flickable
ScrollBar {
    id: root

    property bool attachedFlickableMoving: false
    property alias handleColor: scrollBarRect.color

    active: {
        if (root.orientation === Qt.Horizontal)
            return visible
        else
            return hovered || pressed || attachedFlickableMoving
    }
    hoverEnabled: true
    orientation: Qt.Vertical

    topPadding: root.orientation === Qt.Vertical ? 2 : 0
    leftPadding: root.orientation === Qt.Horizontal ? 2 : 0
    bottomPadding: 2
    rightPadding: 2

    contentItem: Rectangle {
        id: scrollBarRect

        implicitHeight: JamiTheme.scrollBarHandleSize
        implicitWidth: JamiTheme.scrollBarHandleSize
        radius: width / 2
        color: pressed ? Qt.darker(JamiTheme.scrollBarHandleColor, 2.0) :
                         JamiTheme.scrollBarHandleColor
        opacity: 0

        states: State {
            name: "active"
            when: root.policy === ScrollBar.AlwaysOn ||
                  (root.active && root.size < 1.0)
            PropertyChanges {
                target: root.contentItem
                opacity: 1
            }
        }

        transitions: Transition {
            from: "active"
            SequentialAnimation {
                PauseAnimation { duration: JamiTheme.longFadeDuration }
                NumberAnimation { target: root.contentItem
                    duration: JamiTheme.shortFadeDuration
                    property: "opacity"
                    to: 0.0
                }
            }
        }
    }

    background: Rectangle {
        implicitHeight: scrollBarRect.implicitHeight
        implicitWidth: scrollBarRect.implicitWidth
        color: JamiTheme.transparentColor
        radius: width / 2
    }
}
