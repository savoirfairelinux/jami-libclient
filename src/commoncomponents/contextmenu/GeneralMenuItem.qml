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
import QtQuick.Layouts

import net.jami.Constants 1.1

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
    property bool autoTextSizeAdjustment: true
    property BaseContextMenu parentMenu

    property int itemPreferredWidth: JamiTheme.menuItemsPreferredWidth
    property int itemPreferredHeight: JamiTheme.menuItemsPreferredHeight
    property int leftBorderWidth: JamiTheme.menuItemsCommonBorderWidth
    property int rightBorderWidth: JamiTheme.menuItemsCommonBorderWidth

    property int itemImageLeftMargin: 24
    property int itemTextMargin: 20

    signal clicked

    contentItem: AbstractButton {
        id: menuItemContentRect

        background: Rectangle {
            id: background

            anchors.fill: parent
            anchors.leftMargin: 1
            anchors.rightMargin: 1

            color: menuItemContentRect.hovered ?
                       JamiTheme.hoverColor : JamiTheme.backgroundColor
        }

        anchors.fill: parent

        RowLayout {
            spacing: 0

            anchors.fill: menuItemContentRect

            ResponsiveImage {
                id: contextMenuItemImage

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.leftMargin: status === Image.Ready ? itemImageLeftMargin : 0

                visible: status === Image.Ready

                color: iconColor !== "" ? iconColor : JamiTheme.textColor
                opacity: 0.7
            }

            Text {
                id: contextMenuItemText

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.leftMargin: contextMenuItemImage.status === Image.Ready ?
                                       itemTextMargin : itemTextMargin / 2
                Layout.rightMargin: contextMenuItemImage.status === Image.Ready ?
                                        itemTextMargin : itemTextMargin / 2
                Layout.preferredHeight: itemPreferredHeight
                Layout.fillWidth: true

                text: itemName
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                TextMetrics {
                    id: contextMenuItemTextMetrics

                    font: contextMenuItemText.font
                    text: contextMenuItemText.text

                    onBoundingRectChanged: {
                        var sizeToCompare = itemPreferredWidth -
                                (contextMenuItemImage.source.toString().length > 0 ?
                                     itemTextMargin + itemImageLeftMargin + contextMenuItemImage.width :
                                     itemTextMargin / 2)
                        if (autoTextSizeAdjustment
                                && boundingRect.width > sizeToCompare) {
                            if (boundingRect.width > JamiTheme.contextMenuItemTextMaxWidth) {
                                itemPreferredWidth += JamiTheme.contextMenuItemTextMaxWidth
                                        - JamiTheme.contextMenuItemTextPreferredWidth
                                        + itemTextMargin
                                contextMenuItemText.elide = Text.ElideRight
                            } else
                                itemPreferredWidth += boundingRect.width + itemTextMargin
                                        - sizeToCompare
                        }
                    }
                }
            }
        }

        onReleased: {
            menuItem.clicked()
            parentMenu.close()
        }
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
