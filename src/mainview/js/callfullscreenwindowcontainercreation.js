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

// Global call full screen window container, object variable for creation.
var callFullScreenWindowContainerComponent
var callFullScreenWindowContainerObject

function createvideoCallFullScreenWindowContainerObject() {
    if (callFullScreenWindowContainerObject)
        return
    callFullScreenWindowContainerComponent = Qt.createComponent(
                "../components/VideoCallFullScreenWindowContainer.qml")
    if (callFullScreenWindowContainerComponent.status === Component.Ready)
        finishCreation()
    else if (callFullScreenWindowContainerComponent.status === Component.Error)
        console.log("Error loading component:",
                    callFullScreenWindowContainerComponent.errorString())
}

function finishCreation() {
    callFullScreenWindowContainerObject
            = callFullScreenWindowContainerComponent.createObject()
    if (callFullScreenWindowContainerObject === null) {
        // Error Handling.
        console.log("Error creating video call full screen window container object")
    }

    // Signal connection.
    callFullScreenWindowContainerObject.onClosing.connect(
                destroyVideoCallFullScreenWindowContainer)
}

function checkIfVisible() {
    if (!callFullScreenWindowContainerObject)
        return false
    return callFullScreenWindowContainerObject.visible
}

function setAsContainerChild(obj) {
    if (callFullScreenWindowContainerObject)
        callFullScreenWindowContainerObject.setAsChild(obj)
}

// Destroy and reset callFullScreenWindowContainerObject when window is closed.
function destroyVideoCallFullScreenWindowContainer() {
    if (!callFullScreenWindowContainerObject)
        return
    callFullScreenWindowContainerObject.destroy()
    callFullScreenWindowContainerObject = false
}

function showVideoCallFullScreenWindowContainer() {
    if (callFullScreenWindowContainerObject) {

        // Hack: show first, then showFullScreen to make sure that the showFullScreen
        // display on the correct screen.
        callFullScreenWindowContainerObject.show()
        callFullScreenWindowContainerObject.showFullScreen()
    }
}

function closeVideoCallFullScreenWindowContainer() {
    if (callFullScreenWindowContainerObject)
        callFullScreenWindowContainerObject.close()
}
