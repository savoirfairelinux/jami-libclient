/**
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
 * Global mediahandler picker component, object variable for creation.
 */
var mediahandlerPickerComponent
var mediahandlerPickerObject

function createMediaHandlerPickerObjects(parent) {
    if (mediahandlerPickerObject) {
        /*
         * If already created, reset parameters, since object cannot be destroyed.
         */
        mediahandlerPickerObject.parent = parent
        return
    }
    mediahandlerPickerComponent = Qt.createComponent(
                "../components/MediaHandlerPicker.qml")
    if (mediahandlerPickerComponent.status === Component.Ready)
        finishCreation(parent)
    else if (mediahandlerPickerComponent.status === Component.Error)
        console.log("Error loading component:",
                    mediahandlerPickerComponent.errorString())
}

function finishCreation(parent) {
    mediahandlerPickerObject = mediahandlerPickerComponent.createObject(parent)
    if (mediahandlerPickerObject === null) {
        /*
         * Error Handling.
         */
        console.log("Error creating object for mediahandler picker")
    } else {
        mediahandlerPickerObject.x = Qt.binding(function(){
            return parent.width/2 - mediahandlerPickerObject.width / 2})
        mediahandlerPickerObject.y = Qt.binding(function(){
            return parent.height/2 - mediahandlerPickerObject.height / 2})
    }
}

function openMediaHandlerPicker() {
    if (mediahandlerPickerObject)
        mediahandlerPickerObject.open()
}

function closeMediaHandlerPicker() {
    if (mediahandlerPickerObject)
        mediahandlerPickerObject.close()
}
