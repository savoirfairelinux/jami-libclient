/**
 * Copyright (C) 2021-2022 Savoir-faire Linux Inc.
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
 * Global pluginhandler picker component, object variable for creation.
 */
var pluginhandlerPickerComponent
var pluginhandlerPickerObject

function createPluginHandlerPickerObjects(parent, isCall) {
    if (pluginhandlerPickerObject) {
        /*
         * If already created, reset parameters, since object cannot be destroyed.
         */
        pluginhandlerPickerObject.parent = parent
        return
    }
    pluginhandlerPickerComponent = Qt.createComponent(
                "../components/PluginHandlerPicker.qml")
    if (pluginhandlerPickerComponent.status === Component.Ready)
        finishCreation(parent, isCall)
    else if (pluginhandlerPickerComponent.status === Component.Error)
        console.log("Error loading component:",
                    pluginhandlerPickerComponent.errorString())
}

function finishCreation(parent, isCall) {
    pluginhandlerPickerObject = pluginhandlerPickerComponent.createObject(parent)
    if (pluginhandlerPickerObject === null) {
        /*
         * Error Handling.
         */
        console.log("Error creating object for pluginhandler picker")
    } else {
        pluginhandlerPickerObject.x = Qt.binding(function(){
            return parent.width/2 - pluginhandlerPickerObject.width / 2})
        pluginhandlerPickerObject.y = Qt.binding(function(){
            return parent.height/2 - pluginhandlerPickerObject.height / 2})
    }
    pluginhandlerPickerObject.isCall = isCall
}


/*
 * Put pluginhandler picker in the middle of container.
 */
function calculateCurrentGeo(containerX, containerY) {
    if (pluginhandlerPickerObject) {
        pluginhandlerPickerObject.x = containerX - pluginhandlerPickerObject.width / 2
        pluginhandlerPickerObject.y = containerY - pluginhandlerPickerObject.height / 2
    }
}

function openPluginHandlerPicker() {
    if (pluginhandlerPickerObject)
        pluginhandlerPickerObject.open()
}

function closePluginHandlerPicker() {
    if (pluginhandlerPickerObject)
        pluginhandlerPickerObject.close()
}
