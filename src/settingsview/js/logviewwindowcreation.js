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
var logViewWindowComponent
var logViewWindowObject

function createlogViewWindowObject() {
    if (logViewWindowObject)
        return
    logViewWindowComponent = Qt.createComponent(
                "../components/LogsView.qml")
    if (logViewWindowComponent.status === Component.Ready)
        finishCreation()
    else if (logViewWindowComponent.status === Component.Error)
        console.log("Error loading component:",
                    logViewWindowComponent.errorString())
}

function finishCreation() {
    logViewWindowObject = logViewWindowComponent.createObject()
    if (logViewWindowObject === null) {
        // Error Handling.
        console.log("Error creating select screen object")
    }

    // Signal connection.
    logViewWindowObject.onClosing.connect(destroyLogViewWindow)
}

function showLogViewWindow() {
    logViewWindowObject.show()

    var screen = logViewWindowObject.screen
    logViewWindowObject.x = screen.virtualX +
            (screen.width - logViewWindowObject.width) / 2
    logViewWindowObject.y = screen.virtualY +
            (screen.height - logViewWindowObject.height) / 2
}

// Destroy and reset selectScreenWindowObject when window is closed.
function destroyLogViewWindow() {
    if(!logViewWindowObject)
        return
    logViewWindowObject.destroy()
    logViewWindowObject = false
}
