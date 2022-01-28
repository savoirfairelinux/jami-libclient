/*
 * Copyright (C) 2020-2022 Savoir-faire Linux Inc.
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
var selectScreenWindowComponent
var selectScreenWindowObject

function createSelectScreenWindowObject() {
    if (selectScreenWindowObject)
        return
    selectScreenWindowComponent = Qt.createComponent(
                "../components/SelectScreen.qml")
    if (selectScreenWindowComponent.status === Component.Ready)
        finishCreation()
    else if (selectScreenWindowComponent.status === Component.Error)
        console.log("Error loading component:",
                    selectScreenWindowComponent.errorString())
}

function finishCreation() {
    selectScreenWindowObject = selectScreenWindowComponent.createObject()
    if (selectScreenWindowObject === null) {
        // Error Handling.
        console.log("Error creating select screen object")
    }

    // Signal connection.
    selectScreenWindowObject.onClosing.connect(destroySelectScreenWindow)
}

function showSelectScreenWindow(previewId) {
    console.log("previewId", previewId)
    selectScreenWindowObject.currentPreview = previewId
    selectScreenWindowObject.show()

    var screen = selectScreenWindowObject.screen
    selectScreenWindowObject.x = screen.virtualX +
            (screen.width - selectScreenWindowObject.width) / 2
    selectScreenWindowObject.y = screen.virtualY +
            (screen.height - selectScreenWindowObject.height) / 2
}

// Destroy and reset selectScreenWindowObject when window is closed.
function destroySelectScreenWindow() {
    if(!selectScreenWindowObject)
        return
    selectScreenWindowObject.destroy()
    selectScreenWindowObject = false
}
