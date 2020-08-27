/**
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import net.jami.Models 1.0

import "../../commoncomponents"

import "../../commoncomponents/js/contextmenugenerator.js" as ContextMenuGenerator
import "../js/videodevicecontextmenuitemcreation.js" as VideoDeviceContextMenuItemCreation
import "../js/selectscreenwindowcreation.js" as SelectScreenWindowCreation
import "../js/screenrubberbandcreation.js" as ScreenRubberBandCreation

Item {
    id: root

    property bool isSIP: false
    property bool isPaused: false
    property bool isAudioOnly: false
    property bool isRecording: false

    signal pluginItemClicked
    signal transferCallButtonClicked

    function openMenu(){
        if (isSIP){
            ContextMenuGenerator.addMenuItem(isPaused ? qsTr("Resume call") : qsTr("Hold call"),
                                             isPaused ?
                                                 "qrc:/images/icons/play_circle_outline-24px.svg" :
                                                 "qrc:/images/icons/pause_circle_outline-24px.svg",
                                             function (){
                                                 CallAdapter.holdThisCallToggle()
                                             })
            ContextMenuGenerator.addMenuItem(qsTr("Sip Input Panel"),
                                             "qrc:/images/icons/ic_keypad.svg",
                                             function (){
                                                 sipInputPanel.open()
                                             })
            ContextMenuGenerator.addMenuItem(qsTr("Transfer call"),
                                             "qrc:/images/icons/phone_forwarded-24px.svg",
                                             function (){
                                                 root.transferCallButtonClicked()
                                             })

            ContextMenuGenerator.addMenuSeparator()
        }

        if (!isAudioOnly) {
            ContextMenuGenerator.addMenuItem(isRecording ? qsTr("Stop recording") :
                                                           qsTr("Start recording"),
                                             "qrc:/images/icons/ic_video_call_24px.svg",
                                             function (){
                                                  CallAdapter.recordThisCallToggle()
                                             })
            ContextMenuGenerator.addMenuItem(videoCallPage.isFullscreen ? qsTr("Exit full screen") :
                                                                          qsTr("Full screen mode"),
                                             videoCallPage.isFullscreen ?
                                                 "qrc:/images/icons/close_fullscreen-24px.svg" :
                                                 "qrc:/images/icons/open_in_full-24px.svg",
                                             function (){
                                                  videoCallPageRect.needToShowInFullScreen()
                                             })

            ContextMenuGenerator.addMenuSeparator()

            generateDeviceMenuItem()

            ContextMenuGenerator.addMenuSeparator()

            ContextMenuGenerator.addMenuItem(qsTr("Share entire screen"),
                                             "qrc:/images/icons/screen_share-24px.svg",
                                             function (){
                                                 if (Qt.application.screens.length === 1) {
                                                     AvAdapter.shareEntireScreen(0)
                                                 } else {
                                                     SelectScreenWindowCreation.createSelectScreenWindowObject()
                                                     SelectScreenWindowCreation.showSelectScreenWindow()
                                                 }
                                             })
            ContextMenuGenerator.addMenuItem(qsTr("Share screen area"),
                                             "qrc:/images/icons/screen_share-24px.svg",
                                             function (){
                                                 if (Qt.application.screens.length === 1) {
                                                     ScreenRubberBandCreation.createScreenRubberBandWindowObject(
                                                                 null, 0)
                                                     ScreenRubberBandCreation.showScreenRubberBandWindow()
                                                 } else {
                                                     SelectScreenWindowCreation.createSelectScreenWindowObject(true)
                                                     SelectScreenWindowCreation.showSelectScreenWindow()
                                                 }
                                             })
            ContextMenuGenerator.addMenuItem(qsTr("Share file"),
                                             "qrc:/images/icons/insert_photo-24px.svg",
                                             function (){
                                                  jamiFileDialog.open()
                                             })
        }

        ContextMenuGenerator.addMenuItem(qsTr("Toggle plugin"),
                                         "qrc:/images/icons/extension_24dp.svg",
                                         function (){
                                              root.pluginItemClicked()
                                         })

        root.height = ContextMenuGenerator.getMenu().height
        root.width = ContextMenuGenerator.getMenu().width
        ContextMenuGenerator.getMenu().open()
    }

    function generateDeviceMenuItem() {
        var deviceContextMenuInfoMap = AvAdapter.populateVideoDeviceContextMenuItem()

        /*
         * Somehow, the map size is undefined, so use this instead.
         */
        var mapSize = deviceContextMenuInfoMap["size"]

        if (mapSize === 0)
            VideoDeviceContextMenuItemCreation.createVideoDeviceContextMenuItemObjects(
                        qsTr("No video device"), false)

        for (var deviceName in deviceContextMenuInfoMap) {
            if (deviceName === "size")
                continue
            VideoDeviceContextMenuItemCreation.createVideoDeviceContextMenuItemObjects(
                        deviceName, deviceContextMenuInfoMap[deviceName])
        }
    }

    JamiFileDialog {
        id: jamiFileDialog

        mode: JamiFileDialog.Mode.OpenFile

        onAccepted: {
            // No need to trim file:///.
            AvAdapter.shareFile(jamiFileDialog.file)
        }
    }

    Component.onCompleted: {
        ContextMenuGenerator.createBaseContextMenuObjects(root)
        VideoDeviceContextMenuItemCreation.setVideoContextMenuObject(ContextMenuGenerator.getMenu())

        ContextMenuGenerator.getMenu().closed.connect(function (){
            VideoDeviceContextMenuItemCreation.removeCreatedItems()
        })
    }

    /* TODO: In the future we want to implement this

    GeneralMenuItem {
        id: advancedInfosItem

        itemName: qsTr("Advanced informations")
        iconSource: "qrc:/images/icons/info-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            root.close()
        }
    }*/
}

