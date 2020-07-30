
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
import QtGraphicalEffects 1.0
import net.jami.Models 1.0

/*
 * General menu item.
 * Can control top, bottom, left, right border width.
 * Use onClicked slot to simulate item click event.
 * Can have image icon at the left of the text.
 */
MenuItem {
    id: menuItem

    property string itemName: ""
    property string iconSource: ""
    property int preferredWidth: 220
    property int preferredHeight: 48
    property int topBorderWidth: 0
    property int bottomBorderWidth: 0
    property int leftBorderWidth: 0
    property int rightBorderWidth: 0

    signal clicked

    contentItem: Rectangle {
        id: menuItemContentRect

        anchors.fill: parent
        Image {
            id: contextMenuItemImage

            anchors.left: menuItemContentRect.left
            anchors.leftMargin: (visible ? 24 : 0)
            anchors.verticalCenter: menuItemContentRect.verticalCenter

            width: (visible ? 24 : 0)
            height: (visible ? 24 : 0)

            visible: false
            fillMode: Image.PreserveAspectFit
            mipmap: true
            opacity: 0.7
        }

        Text {
            id: contextMenuItemText

            anchors.left: contextMenuItemImage.right
            anchors.leftMargin: 20
            anchors.verticalCenter: menuItemContentRect.verticalCenter
            width: textMetrics.boundingRect.width
            height: 30

            TextMetrics {
                id: textMetrics
                font: contextMenuItemText.font
                elide: Text.ElideRight
                elideWidth: contextMenuItemImage.visible ? (preferredWidth - contextMenuItemImage.width - 58) : preferredWidth - 24
                text: itemName
            }

            text: textMetrics.elidedText
            font.pointSize: JamiTheme.textFontSize
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        color: "transparent"
    }

    onIconSourceChanged: {
        if (iconSource !== "") {
            contextMenuItemImage.source = iconSource
            contextMenuItemImage.visible = true
        }
    }

    background: Rectangle {
        id: contextMenuBackgroundRect

        anchors.fill: parent
        anchors.topMargin: topBorderWidth
        anchors.bottomMargin: bottomBorderWidth
        anchors.leftMargin: leftBorderWidth
        anchors.rightMargin: rightBorderWidth

        implicitWidth: preferredWidth
        implicitHeight: preferredHeight

        border.width: 0
        color: menuItem.down ? JamiTheme.releaseColor : "white"

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onPressed: {
                contextMenuBackgroundRect.color = JamiTheme.pressColor
            }
            onReleased: {
                contextMenuBackgroundRect.color = JamiTheme.releaseColor
                menuItem.clicked()
            }
            onEntered: {
                contextMenuBackgroundRect.color = JamiTheme.hoverColor
            }
            onExited: {
                contextMenuBackgroundRect.color = "white"
            }
        }

        CustomBorder {
            commonBorder: false
            lBorderwidth: leftBorderWidth
            rBorderwidth: rightBorderWidth
            tBorderwidth: topBorderWidth
            bBorderwidth: bottomBorderWidth
            borderColor: JamiTheme.tabbarBorderColor
        }
    }
}
