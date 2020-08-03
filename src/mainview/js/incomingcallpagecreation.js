
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
 * Global incomingCallPage storage map<accountId, map<convUid, callwindowpointer>>.
 */
let incomingCallPageWindowMap = new Map()


/*
 * Global incomingCallPage component, object variable for creation.
 */
var incomingCallPageWindowComponent
var incomingCallPageWindowObject

function createincomingCallPageWindowObjects(accountId, convUid) {


    /*
     * Check if the corrsponding call exists or not.
     */
    if (incomingCallPageWindowMap.has(accountId)) {
        if (incomingCallPageWindowMap.get(accountId).has(convUid)) {
            return
        }
    }

    incomingCallPageWindowComponent = Qt.createComponent(
                "../components/IncomingCallPage.qml")
    if (incomingCallPageWindowComponent.status === Component.Ready)
        finishCreation(accountId, convUid)
    else if (incomingCallPageWindowComponent.status === Component.Error)
        console.log("Error loading component:",
                    incomingCallPageWindowComponent.errorString())
}

function finishCreation(accountId, convUid) {
    incomingCallPageWindowObject = incomingCallPageWindowComponent.createObject(
                )
    if (incomingCallPageWindowObject === null) {


        /*
         * Error Handling.
         */
        console.log("Error creating object for accountId" + accountId)
    }

    incomingCallPageWindowObject.responsibleConvUid = convUid
    incomingCallPageWindowObject.responsibleAccountId = accountId
    incomingCallPageWindowObject.updateUI()


    /*
     * Record in map.
     */
    if (incomingCallPageWindowMap.has(accountId)) {
        incomingCallPageWindowMap.get(accountId).set(
                    convUid, incomingCallPageWindowObject)
    } else {
        let incomingCallPageWindowTempMap = new Map()
        incomingCallPageWindowTempMap.set(convUid, incomingCallPageWindowObject)
        incomingCallPageWindowMap.set(accountId, incomingCallPageWindowTempMap)
    }
}

function showIncomingCallPageWindow(accountId, convUid) {
    if (incomingCallPageWindowMap.has(accountId)) {
        if (incomingCallPageWindowMap.get(accountId).has(convUid)) {
            if (!incomingCallPageWindowMap.get(accountId).get(
                        convUid).visible) {
                incomingCallPageWindowMap.get(accountId).get(convUid).show()
                incomingCallPageWindowMap.get(accountId).get(
                            convUid).updatePositionToRightBottom()
            }
        }
    }
}

function closeIncomingCallPageWindow(accountId, convUid) {
    if (incomingCallPageWindowMap.has(accountId)) {
        let incomingCallPageWindowTempMap = incomingCallPageWindowMap.get(
                accountId)
        if (incomingCallPageWindowTempMap.has(convUid)) {
            var incomingCallPageWindow = incomingCallPageWindowTempMap.get(
                        convUid)


            /*
             * Close incomingCallPageWindow and clear the memory
             */
            incomingCallPageWindow.close()
            incomingCallPageWindow.destroy()
            incomingCallPageWindowTempMap.delete(convUid)
            if (incomingCallPageWindowTempMap.size === 0) {
                incomingCallPageWindowMap.delete(accountId)
            } else {
                incomingCallPageWindowMap.set(accountId,
                                              incomingCallPageWindowTempMap)
            }
        }
    }
}
