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

import net.jami.Constants 1.0

import "../../commoncomponents"

// General menu item.
// Can control top, bottom, left, right border width.
// Use onClicked slot to simulate item click event.
// Can have image icon at the left of the text.
MenuItem {
    id: menuItem

    property string itemName: ""
    property alias iconSource: contextMenuItemImage.source
    property string iconColor: ""
    property bool canTrigger: true
    property bool addMenuSeparatorAfter: false
    property BaseContextMenu parentMenu

    property int itemPreferredWidth: JamiTheme.menuItemsPreferredWidth
    property int itemPreferredHeight: JamiTheme.menuItemsPreferredHeight
    property int leftBorderWidth: JamiTheme.menuItemsCommonBorderWidth
    property int rightBorderWidth: JamiTheme.menuItemsCommonBorderWidth

    signal clicked

    contentItem: AbstractButton {
        id: menuItemContentRect

        background: Rectangle {
            id: background
            anchors.fill: parent
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            color: "transparent"
        }

        anchors.fill: parent

        ResponsiveImage {
            id: contextMenuItemImage

            anchors.left: status === Image.Ready ? menuItemContentRect.left : undefined
            anchors.leftMargin: (status === Image.Ready ? 24 : 0)
            anchors.verticalCenter: menuItemContentRect.verticalCenter

            color: iconColor !== "" ? iconColor : JamiTheme.textColor

            smooth: true
            opacity: 0.7
        }

        Text {
            id: contextMenuItemText

            anchors.left: contextMenuItemImage.status === Image.Ready ?
                              contextMenuItemImage.right : menuItemContentRect.left
            anchors.leftMargin: contextMenuItemImage.status === Image.Ready ? 20 : 10
            anchors.verticalCenter: menuItemContentRect.verticalCenter

            height: itemPreferredHeight

            text: itemName
            color: JamiTheme.textColor
            wrapMode: Text.WordWrap
            font.pointSize: JamiTheme.textFontSize
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        onReleased: {
            menuItem.clicked()
            parentMenu.close()
        }

        states: [
            State {
                name: "hovered"; when: hovered
                PropertyChanges { target: background; color: JamiTheme.hoverColor }
            },
            State {
                name: "normal"; when: !hovered
                PropertyChanges { target: background; color: JamiTheme.backgroundColor }
            }
        ]
    }

    highlighted: true

    background: Rectangle {
        id: contextMenuBackgroundRect

        anchors.fill: parent
        anchors.leftMargin: leftBorderWidth
        anchors.rightMargin: rightBorderWidth

        implicitWidth: itemPreferredWidth
        implicitHeight: itemPreferredHeight

        border.width: 0

        CustomBorder {
            commonBorder: false
            lBorderwidth: leftBorderWidth
            rBorderwidth: rightBorderWidth
            tBorderwidth: 0
            bBorderwidth: 0
            borderColor: JamiTheme.tabbarBorderColor
        }
    }
}
