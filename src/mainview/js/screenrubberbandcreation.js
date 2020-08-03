
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
 * Global screen rubber band window component, object variable for creation.
 */
var screenRubberBandWindowComponent
var screenRubberBandWindowObject

function createScreenRubberBandWindowObject(parent, screenNumber) {
    if (screenRubberBandWindowObject)
        return
    screenRubberBandWindowComponent = Qt.createComponent(
                "../components/ScreenRubberBand.qml")
    if (screenRubberBandWindowComponent.status === Component.Ready)
        finishCreation(parent, screenNumber)
    else if (screenRubberBandWindowComponent.status === Component.Error)
        console.log("Error loading component:",
                    screenRubberBandWindowComponent.errorString())
}

function finishCreation(parent, screenNumber) {
    screenRubberBandWindowObject = screenRubberBandWindowComponent.createObject(
                parent)
    if (screenRubberBandWindowObject === null) {


        /*
         * Error Handling.
         */
        console.log("Error creating screen rubber band object")
    }

    screenRubberBandWindowObject.screenNumber = screenNumber
    screenRubberBandWindowObject.screen = Qt.application.screens[screenNumber]


    /*
     * Signal connection.
     */
    screenRubberBandWindowObject.onClosing.connect(
                destoryScreenRubberBandWindow)
}

function showScreenRubberBandWindow() {
    screenRubberBandWindowObject.showFullScreen()
}


/*
 * Destroy and reset screenRubberBandWindowObject when window is closed.
 */
function destoryScreenRubberBandWindow() {
    if (!screenRubberBandWindowObject)
        return
    screenRubberBandWindowObject.destroy()
    screenRubberBandWindowObject = false
}

function connectOnClosingEvent(func) {
    if (screenRubberBandWindowObject)
        screenRubberBandWindowObject.onClosing.connect(func)
}
