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

// Global base context menu, object variable for creation.
var baseContextMenuComponent
var baseContextMenuObject
var menuItemList = []
var menuDefaultSeparatorHeight = 8

function createBaseContextMenuObjects(parent) {
    // If already created, return, since object cannot be destroyed.
    if (baseContextMenuObject)
        return

    baseContextMenuComponent = Qt.createComponent("../BaseContextMenu.qml")
    if (baseContextMenuComponent.status === Component.Ready)
        finishCreation(parent)
    else if (baseContextMenuComponent.status === Component.Error)
        console.log("Error loading component:",
                    baseContextMenuComponent.errorString())
}

function finishCreation(parent) {
    baseContextMenuObject = baseContextMenuComponent.createObject(parent)
    if (baseContextMenuObject === null) {
        // Error Handling.
        console.log("Error creating object for base context menu")
    }

    baseContextMenuObject.closed.connect(function () {
        // Remove the menu items when hidden.
        for (var i = 0; i < menuItemList.length; i++) {
            baseContextMenuObject.removeItem(menuItemList[i])
        }
    })

    baseContextMenuObject.aboutToShow.connect(function () {
        // Add default separator at the bottom.
        addMenuSeparator(menuDefaultSeparatorHeight, "transparent")
    })
}

function initMenu(preferedMenuItemSize, defaultSeparatorHeight) {
    // Clear any existing items in the menu.
    for (var i = 0; i < menuItemList.length; i++) {
        baseContextMenuObject.removeItem(menuItemList[i])
    }

    if (preferedMenuItemSize) {
        baseContextMenuObject.menuItemsPreferredWidth = preferedMenuItemSize.width
        baseContextMenuObject.menuItemsPreferredHeight = preferedMenuItemSize.height
    }

    if (defaultSeparatorHeight)
        menuDefaultSeparatorHeight = defaultSeparatorHeight
}

function addMenuSeparator(separatorHeight, separatorColor) {
    var menuSeparatorObject
    var menuSeparatorComponent = Qt.createComponent(
                "../GeneralMenuSeparator.qml")
    if (menuSeparatorComponent.status === Component.Ready) {
        baseContextMenuObject.generalMenuSeparatorCount++
        menuSeparatorObject = menuSeparatorComponent.createObject(
                    null, {
                        "preferredWidth": baseContextMenuObject.menuItemsPreferredWidth,
                        "preferredHeight": separatorHeight ? separatorHeight : 1
                    })
        if (separatorColor)
            menuSeparatorObject.separatorColor = separatorColor
    } else if (menuSeparatorComponent.status === Component.Error)
        console.log("Error loading component:",
                    menuSeparatorComponent.errorString())
    if (menuSeparatorObject !== null) {
        baseContextMenuObject.addItem(menuSeparatorObject)

        menuItemList.push(menuSeparatorObject)
    } else {
        // Error handling.
        console.log("Error creating object")
    }
}

function addMenuItem(itemName, iconSource, onClickedCallback, iconColor) {
    if (!baseContextMenuObject.count) {
        // Add default separator at the top.
        addMenuSeparator(menuDefaultSeparatorHeight, "transparent")
    }

    var menuItemObject
    var menuItemComponent = Qt.createComponent("../GeneralMenuItem.qml")
    if (menuItemComponent.status === Component.Ready) {
        menuItemObject = menuItemComponent.createObject(
                    null, {
                        "itemName": itemName,
                        "iconSource": iconSource,
                        "iconColor": iconColor,
                        "preferredWidth": baseContextMenuObject.menuItemsPreferredWidth,
                        "preferredHeight": baseContextMenuObject.menuItemsPreferredHeight,
                        "leftBorderWidth": baseContextMenuObject.commonBorderWidth,
                        "rightBorderWidth": baseContextMenuObject.commonBorderWidth
                    })
    } else if (menuItemComponent.status === Component.Error)
        console.log("Error loading component:", menuItemComponent.errorString())
    if (menuItemObject !== null) {
        menuItemObject.clicked.connect(function () {
            var callback = function () {
                onClickedCallback()
                baseContextMenuObject.onVisibleChanged.disconnect(callback)
                baseContextMenuObject.close()
            }

            baseContextMenuObject.onVisibleChanged.connect(callback)
            baseContextMenuObject.visible = false
        })
        menuItemObject.icon.color = "green"

        baseContextMenuObject.addItem(menuItemObject)

        menuItemList.push(menuItemObject)
    } else {
        // Error handling.
        console.log("Error creating object")
    }
}

function getMenu() {
    return baseContextMenuObject
}
