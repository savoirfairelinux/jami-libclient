
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


/*
 * Global storage for created video device context menu item,
 * will be cleared once context menu is closed.
 */
var itemArray = []


/*
 * Global videoDeviceContextMenuItem component, object variable for creation.
 */
var videoContextMenuObject
var videoDeviceContextMenuItemComponent
var videoDeviceContextMenuItemObject


/*
 * Init videoContextMenuObject.
 */
function setVideoContextMenuObject(obj) {
    videoContextMenuObject = obj
}

function createVideoDeviceContextMenuItemObjects(deviceName, setChecked) {

    videoDeviceContextMenuItemComponent = Qt.createComponent(
                "../components/VideoCallPageContextMenuDeviceItem.qml")
    if (videoDeviceContextMenuItemComponent.status === Component.Ready)
        finishCreation(deviceName, setChecked)
    else if (videoDeviceContextMenuItemComponent.status === Component.Error)
        console.log("Error loading component:",
                    videoDeviceContextMenuItemComponent.errorString())
}

function finishCreation(deviceName, setChecked) {
    videoDeviceContextMenuItemObject = videoDeviceContextMenuItemComponent.createObject()
    if (videoDeviceContextMenuItemObject === null) {
        // Error Handling.
        console.log("Error creating video context menu object")
    }

    videoDeviceContextMenuItemObject.leftBorderWidth =
            videoContextMenuObject.commonBorderWidth
    videoDeviceContextMenuItemObject.rightBorderWidth =
            videoContextMenuObject.commonBorderWidth
    videoDeviceContextMenuItemObject.itemName = deviceName
    videoDeviceContextMenuItemObject.checkable = true
    videoDeviceContextMenuItemObject.checked = setChecked
    videoDeviceContextMenuItemObject.contextMenuPreferredWidth = videoContextMenuObject.implicitWidth


    /*
     * Push into the storage array, and insert it into context menu.
     */
    itemArray.push(videoDeviceContextMenuItemObject)
    videoContextMenuObject.addItem(videoDeviceContextMenuItemObject)

    videoDeviceContextMenuItemObject.clicked.connect(function () {
        videoContextMenuObject.close()
    })
}

function removeCreatedItems() {
    var arrayLength = itemArray.length
    for (var i = 0; i < arrayLength; i++) {
        videoContextMenuObject.removeItem(itemArray[i])
        itemArray[i].destroy()
    }
    itemArray = []
}
