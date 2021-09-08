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
import Qt5Compat.GraphicalEffects

import net.jami.Constants 1.1

Menu {
    id: root

    property int menuPreferredWidth: 0
    property int menuItemsPreferredHeight: 0
    property int menuSeparatorPreferredHeight: 0

    property GeneralMenuSeparator menuTopBorder: GeneralMenuSeparator {
        separatorPreferredWidth: menuPreferredWidth ?
                                     menuPreferredWidth : JamiTheme.menuItemsPreferredWidth
        separatorPreferredHeight: menuSeparatorPreferredHeight ?
                                      menuSeparatorPreferredHeight : JamiTheme.menuBorderPreferredHeight
        separatorColor: "transparent"
    }

    property GeneralMenuSeparator menuBottomBorder: GeneralMenuSeparator {
        separatorPreferredWidth: menuPreferredWidth ?
                                     menuPreferredWidth : JamiTheme.menuItemsPreferredWidth
        separatorPreferredHeight: menuSeparatorPreferredHeight ?
                                      menuSeparatorPreferredHeight : JamiTheme.menuBorderPreferredHeight
        separatorColor: "transparent"
    }

    property var generalMenuSeparatorList: []

    function loadMenuItems(menuItems) {
        root.addItem(menuTopBorder)

        // use the maximum text width as the preferred width for menu
        for (var j = 0; j < menuItems.length; ++j) {
            var currentItemWidth = menuItems[j].itemPreferredWidth
            if (currentItemWidth !== JamiTheme.menuItemsPreferredWidth
                    && currentItemWidth > menuPreferredWidth)
                menuPreferredWidth = currentItemWidth
        }

        for (var i = 0; i < menuItems.length; ++i) {
            if (menuItems[i].canTrigger) {
                menuItems[i].parentMenu = root
                root.addItem(menuItems[i])

                if (menuPreferredWidth)
                    menuItems[i].itemPreferredWidth = menuPreferredWidth
                if (menuItemsPreferredHeight)
                    menuItems[i].itemPreferredHeight = menuItemsPreferredHeight
            }
            if (menuItems[i].addMenuSeparatorAfter) {
                // If the QML file to be loaded is a local file,
                // you could omit the finishCreation() function
                var menuSeparatorComponent = Qt.createComponent(
                            "GeneralMenuSeparator.qml",
                            Component.PreferSynchronous, root)
                var menuSeparatorComponentObj = menuSeparatorComponent.createObject()
                generalMenuSeparatorList.push(menuSeparatorComponentObj)
                root.addItem(menuSeparatorComponentObj)
            }
        }

        root.addItem(menuBottomBorder)

        root.open()
    }

    onVisibleChanged: {
        if (!visible)
            root.close()
    }

    modal: true
    Overlay.modal: Rectangle {
        color: "transparent"
    }
    font.pointSize: JamiTheme.menuFontSize

    background: Rectangle {
        id: container

        implicitWidth: menuPreferredWidth ? menuPreferredWidth : JamiTheme.menuItemsPreferredWidth

        border.width: JamiTheme.menuItemsCommonBorderWidth
        border.color: JamiTheme.tabbarBorderColor
        color: JamiTheme.backgroundColor

        layer.enabled: true
        layer.effect: DropShadow {
            z: -1
            horizontalOffset: 3.0
            verticalOffset: 3.0
            radius: 16.0
            color: JamiTheme.shadowColor
            transparentBorder: true
        }
    }

    Component.onDestruction: {
        for (var i = 0; i < generalMenuSeparatorList.length; ++i) {
            generalMenuSeparatorList[i].destroy()
        }
    }
}
