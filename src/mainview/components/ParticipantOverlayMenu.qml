/*
 * Copyright (C) 2020-2022 Savoir-faire Linux Inc.
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
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

// Overlay menu for conference moderation
Item {
    id: root

    property string uri: ""
    property bool isLocalMuted: true
    property bool showSetModerator: false
    property bool showUnsetModerator: false
    property bool showModeratorMute: false
    property bool showModeratorUnmute: false
    property bool showMaximize: false
    property bool showMinimize: false
    property bool showHangup: false

    property int shapeHeight: 30
    property int shapeRadius: 8

    property bool isBarLayout: root.width > 220
    property int isSmall: !isBarLayout && (root.height < 100 || root.width < 160)

    property int buttonPreferredSize: 24
    property int iconButtonPreferredSize: 16

    property alias hovered: hover.hovered

    anchors.fill: parent

    HoverHandler { id: hover }

    Loader { sourceComponent: isBarLayout ? barComponent : rectComponent }

    Component {
        id: rectComponent

        Control {
            width: root.width
            height: root.height
            hoverEnabled: false

            background: Rectangle {
                property int buttonsSize: buttonsRect.visibleButtons * 24 + 8 * 2
                property bool isOverlayRect: buttonsSize + 32 > root.width

                color: JamiTheme.darkGreyColorOpacity
                radius: isOverlayRect ? 10 : 0

                anchors.fill: isOverlayRect ? undefined : parent
                anchors.centerIn: parent
                width: isOverlayRect ? buttonsSize + 32 : parent.width
                height: isOverlayRect ? 80 : parent.height
            }

            ParticipantControlLayout {
                id: buttonsRect
                anchors.centerIn: parent
            }
        }
    }

    Component {
        id: barComponent

        Control {
            width: barButtons.implicitWidth
            height: shapeHeight
            hoverEnabled: false

            background: Item {
                clip: true
                Rectangle {
                    color: JamiTheme.darkGreyColorOpacity
                    radius: shapeRadius
                    width: parent.width + 2 * radius
                    height: parent.height + 2 * radius
                    anchors.fill: parent
                    anchors.leftMargin: -radius
                    anchors.topMargin: -radius
                }
            }

            ParticipantControlLayout {
                id: barButtons
            }
        }
    }
}
