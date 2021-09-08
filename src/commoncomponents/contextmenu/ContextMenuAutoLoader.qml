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

import "../../commoncomponents/contextmenu"

Loader {
    id: root

    // Cannot have menuItemsToLoad directly assigned as list<GeneralMenuItem>
    // https://stackoverflow.com/questions/26733011/how-to-declare-list-property-in-qml
    property var menuItemsToLoad
    property int contextMenuItemPreferredWidth: 0
    property int contextMenuItemPreferredHeight: 0
    property int contextMenuSeparatorPreferredHeight: 0

    function openMenu() {
        root.active = true
        root.sourceComponent = menuComponent
    }

    Connections {
        target: root.item
        enabled: root.status === Loader.Ready
        function onClosed() {
            root.active = false
        }
    }

    Component {
        id: menuComponent

        BaseContextMenu {
            id: contextMenu

            Component.onCompleted: {
                contextMenu.menuPreferredWidth = contextMenuItemPreferredWidth
                contextMenu.menuItemsPreferredHeight = contextMenuItemPreferredHeight
                contextMenu.menuSeparatorPreferredHeight = contextMenuSeparatorPreferredHeight
                contextMenu.loadMenuItems(menuItemsToLoad)
            }
        }
    }
}
