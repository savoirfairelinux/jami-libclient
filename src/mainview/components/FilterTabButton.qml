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
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

TabButton {
    id: root

    property var tabBar: undefined
    property alias labelText: label.text
    property alias acceleratorSequence: accelerator.sequence
    property int badgeCount
    signal selected

    hoverEnabled: true
    onClicked: selected()

    Rectangle {
        id: rect

        width: tabBar.width / 2 + 1
        height: tabBar.height
        color: root.hovered ?
                   JamiTheme.hoverColor :
                   JamiTheme.backgroundColor

        Text {
            id: label

            anchors.horizontalCenter: rect.horizontalCenter
            anchors.bottom: rect.bottom
            anchors.bottomMargin: 12

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.pointSize: JamiTheme.textFontSize
            color: Qt.lighter(JamiTheme.textColor, root.down ? 1.0 : 1.5)
        }

        Rectangle {
            id: badgeRect

            anchors.left: label.right
            anchors.leftMargin: 4
            anchors.verticalCenter: label.verticalCenter
            anchors.verticalCenterOffset : -5

            width: 12
            height: 12

            visible: badgeCount > 0

            Text {
                anchors.centerIn: badgeRect
                text: badgeCount > 9 ? "···" : badgeCount
                color: JamiTheme.whiteColor
                font.pointSize: JamiTheme.indicatorFontSize
            }
            radius: 30
            color: JamiTheme.notificationBlue
        }

        Rectangle {
            width: rect.width
            anchors.bottom: rect.bottom
            height: 2
            color: root.down ? JamiTheme.textColor : "transparent"
        }
    }

    Shortcut {
        id: accelerator
        context: Qt.ApplicationShortcut
        enabled: rect.visible
        onActivated: selected()
    }
}
