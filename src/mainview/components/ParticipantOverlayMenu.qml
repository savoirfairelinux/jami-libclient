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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

// Overlay menu for conference moderation
Item {
    id: root

    property string uri: ""
    property string bestName: ""
    property bool isLocalMuted: true
    property bool showSetModerator: false
    property bool showUnsetModerator: false
    property bool showModeratorMute: false
    property bool showModeratorUnmute: false
    property bool showMaximize: false
    property bool showMinimize: false
    property bool showHangup: false
    property bool showLowerHand: false

    property int shapeHeight: 30
    property int shapeRadius: 8

    property bool isBarLayout: root.width > 220
    property int isSmall: !isBarLayout && (root.height < 100 || root.width < 160)

    property int buttonPreferredSize: 24
    property int iconButtonPreferredSize: 16

    property bool hovered: false

    anchors.fill: parent

    Loader { sourceComponent: isBarLayout ? barComponent : rectComponent }

    TextMetrics {
        id: nameTextMetrics
        text: bestName
        font.pointSize: JamiTheme.participantFontSize
    }

    Component {
        id: rectComponent

        Control {
            width: root.width
            height: root.height

            onHoveredChanged: root.hovered = hovered

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

            ColumnLayout {

                anchors.centerIn: parent

                Text {
                    id: bestNameLabel

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredWidth: Math.min(nameTextMetrics.boundingRect.width + 8,
                                                    root.width - 16)
                    Layout.preferredHeight: shapeHeight

                    text: bestName
                    elide: Text.ElideRight
                    color: JamiTheme.whiteColor
                    font.pointSize: JamiTheme.participantFontSize
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }
                ParticipantControlLayout {
                    id: buttonsRect

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredWidth: implicitWidth
                    Layout.preferredHeight: shapeHeight
                }
            }
        }
    }

    Component {
        id: barComponent

        Control {
            width: rowLayout.implicitWidth
            height: shapeHeight

            onHoveredChanged: root.hovered = hovered

            background: Item {
                clip: true
                Rectangle {
                    color: JamiTheme.darkGreyColorOpacity
                    radius: shapeRadius
                    width: parent.width + radius
                    height: parent.height + radius
                    anchors.fill: parent
                    anchors.leftMargin: -radius
                    anchors.topMargin: -radius
                }
            }

            RowLayout {
                id: rowLayout

                spacing: 8

                Text {
                    id: bestNameLabel

                    Layout.leftMargin: 8
                    Layout.preferredWidth: Math.min(nameTextMetrics.boundingRect.width + 8,
                                                    root.width - buttonsRect.implicitWidth - 16)
                    Layout.preferredHeight: shapeHeight

                    text: bestName
                    elide: Text.ElideRight
                    color: JamiTheme.whiteColor
                    font.pointSize: JamiTheme.participantFontSize
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                ParticipantControlLayout {
                    id: buttonsRect

                    Layout.rightMargin: 8
                    Layout.preferredWidth: implicitWidth
                    Layout.preferredHeight: shapeHeight
                }
            }
        }
    }
}
