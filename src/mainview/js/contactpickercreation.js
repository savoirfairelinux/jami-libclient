
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
 * Global contact picker component, object variable for creation.
 */
var contactPickerComponent
var contactPickerObject

function createContactPickerObjects(type, parent) {
    if (contactPickerObject) {


        /*
         * If already created, reset parameters, since object cannot be destroyed.
         */
        contactPickerObject.parent = parent
        contactPickerObject.type = type
        return
    }
    contactPickerComponent = Qt.createComponent(
                "../components/ContactPicker.qml")
    if (contactPickerComponent.status === Component.Ready)
        finishCreation(type, parent)
    else if (contactPickerComponent.status === Component.Error)
        console.log("Error loading component:",
                    contactPickerComponent.errorString())
}

function finishCreation(type, parent) {
    contactPickerObject = contactPickerComponent.createObject(parent, {
                                                                  "type": type
                                                              })
    if (contactPickerObject === null) {


        /*
         * Error Handling.
         */
        console.log("Error creating object for contact picker")
    }
}


/*
 * Put contact picker in the middle of container.
 */
function calculateCurrentGeo(containerX, containerY) {
    if (contactPickerObject) {
        contactPickerObject.x = containerX - contactPickerObject.width / 2
        contactPickerObject.y = containerY - contactPickerObject.height / 2
    }
}

function openContactPicker() {
    if (contactPickerObject)
        contactPickerObject.open()
}

function closeContactPicker() {
    if (contactPickerObject)
        contactPickerObject.close()
}
