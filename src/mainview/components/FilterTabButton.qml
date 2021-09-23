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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Constants 1.1

import "../../commoncomponents"

TabButton {
    id: root

    property alias labelText: label.text
    property alias acceleratorSequence: accelerator.sequence
    property alias badgeCount: badge.count
    signal selected

    hoverEnabled: true
    onClicked: selected()

     Rectangle {
        id: contentRect

        anchors.fill: root

        color: root.hovered ?
                   JamiTheme.hoverColor :
                   JamiTheme.backgroundColor

        RowLayout {
            anchors.horizontalCenter: contentRect.horizontalCenter
            anchors.verticalCenter: contentRect.verticalCenter

            Text {
                id: label

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.bottomMargin: 1

                font.pointSize: JamiTheme.filterItemFontSize
                color: JamiTheme.textColor
                opacity: root.down ? 1.0 : 0.5
            }

            BadgeNotifier {
                id: badge
                size: 20
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
        }
    }

    Rectangle {
        width: contentRect.width
        anchors.bottom: contentRect.bottom
        height: 2
        color: root.down ? JamiTheme.textColor : "transparent"
    }

    Shortcut {
        id: accelerator
        context: Qt.ApplicationShortcut
        enabled: contentRect.visible
        onActivated: selected()
    }
}
