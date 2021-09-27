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

// Global select screen window component, object variable for creation.
var keyboardShortcutTableWindowComponent
var keyboardShortcutTableWindowObject

function createKeyboardShortcutTableWindowObject() {
    if (keyboardShortcutTableWindowObject)
        return
    keyboardShortcutTableWindowComponent = Qt.createComponent(
                "../components/KeyboardShortcutTable.qml")
    if (keyboardShortcutTableWindowComponent.status === Component.Ready)
        finishCreation()
    else if (keyboardShortcutTableWindowComponent.status === Component.Error)
        console.log("Error loading component:",
                    keyboardShortcutTableWindowComponent.errorString())
}

function finishCreation() {
    keyboardShortcutTableWindowObject = keyboardShortcutTableWindowComponent.createObject()
    if (keyboardShortcutTableWindowObject === null) {
        // Error Handling.
        console.log("Error creating select screen object")
    }

    // Signal connection.
    keyboardShortcutTableWindowObject.onClosing.connect(destroyKeyboardShortcutTableWindow)
}

function showKeyboardShortcutTableWindow() {
    keyboardShortcutTableWindowObject.show()

    var screen = keyboardShortcutTableWindowObject.screen
    keyboardShortcutTableWindowObject.x = screen.virtualX +
            (screen.width - keyboardShortcutTableWindowObject.width) / 2
    keyboardShortcutTableWindowObject.y = screen.virtualY +
            (screen.height - keyboardShortcutTableWindowObject.height) / 2
}

// Destroy and reset selectScreenWindowObject when window is closed.
function destroyKeyboardShortcutTableWindow() {
    if(!keyboardShortcutTableWindowObject)
        return
    keyboardShortcutTableWindowObject.destroy()
    keyboardShortcutTableWindowObject = false
}
