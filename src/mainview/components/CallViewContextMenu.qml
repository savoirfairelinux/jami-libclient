/*
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
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

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
        ContextMenuGenerator.initMenu()
        if (isSIP){
            ContextMenuGenerator.addMenuItem(isPaused ? JamiStrings.resume : JamiStrings.hold,
                                             isPaused ?
                                                 "qrc:/images/icons/play_circle_outline-24px.svg" :
                                                 "qrc:/images/icons/pause_circle_outline-24px.svg",
                                             function (){
                                                 CallAdapter.holdThisCallToggle()
                                             })
            ContextMenuGenerator.addMenuItem(JamiStrings.sipInputPanel,
                                             "qrc:/images/icons/ic_keypad.svg",
                                             function (){
                                                 sipInputPanel.open()
                                             })
            ContextMenuGenerator.addMenuItem(JamiStrings.transferCall,
                                             "qrc:/images/icons/phone_forwarded-24px.svg",
                                             function (){
                                                 root.transferCallButtonClicked()
                                             })

            ContextMenuGenerator.addMenuSeparator()
        }

        ContextMenuGenerator.addMenuItem(isRecording ? JamiStrings.stopRec :
                                                       JamiStrings.startRec,
                                         "qrc:/images/icons/av_icons/fiber_manual_record-24px.svg",
                                         function (){
                                              CallAdapter.recordThisCallToggle()
                                         })

        if (isAudioOnly && !isPaused)
            ContextMenuGenerator.addMenuItem(
                        JamiQmlUtils.callIsFullscreen ? JamiStrings.exitFullScreen :
                                                         JamiStrings.fullScreen,
                        JamiQmlUtils.callIsFullscreen ?
                            "qrc:/images/icons/close_fullscreen-24px.svg" :
                            "qrc:/images/icons/open_in_full-24px.svg",
                        function (){
                             callStackView.toggleFullScreen()
                        })

        if (!isAudioOnly && !isPaused) {
            ContextMenuGenerator.addMenuItem(
                        JamiQmlUtils.callIsFullscreen ? JamiStrings.exitFullScreen :
                                                        JamiStrings.fullScreen,
                        JamiQmlUtils.callIsFullscreen ?
                            "qrc:/images/icons/close_fullscreen-24px.svg" :
                            "qrc:/images/icons/open_in_full-24px.svg",
                        function (){
                            callStackView.toggleFullScreen()
                        })

            ContextMenuGenerator.addMenuSeparator()

            generateDeviceMenuItem()

            ContextMenuGenerator.addMenuSeparator()

            ContextMenuGenerator.addMenuItem(JamiStrings.shareScreen,
                                             "qrc:/images/icons/screen_share-24px.svg",
                                             function (){
                                                 if (Qt.application.screens.length === 1) {
                                                     AvAdapter.shareEntireScreen(0)
                                                 } else {
                                                     SelectScreenWindowCreation.createSelectScreenWindowObject()
                                                     SelectScreenWindowCreation.showSelectScreenWindow()
                                                 }
                                             })
            ContextMenuGenerator.addMenuItem(JamiStrings.shareScreenArea,
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
            ContextMenuGenerator.addMenuItem(JamiStrings.shareFile,
                                             "qrc:/images/icons/insert_photo-24px.svg",
                                             function (){
                                                  jamiFileDialog.open()
                                             })
        }

        if (UtilsAdapter.checkShowPluginsButton()) {
            ContextMenuGenerator.addMenuItem(JamiStrings.viewPlugin,
                                             "qrc:/images/icons/extension_24dp.svg",
                                             function (){
                                                  root.pluginItemClicked()
                                             })
        }

        root.height = ContextMenuGenerator.getMenu().height
        root.width = ContextMenuGenerator.getMenu().width
        ContextMenuGenerator.getMenu().open()
    }

    function generateDeviceMenuItem() {
        var deviceContextMenuInfoMap = AvAdapter.populateVideoDeviceContextMenuItem()

        // Somehow, the map size is undefined, so use this instead.
        var mapSize = deviceContextMenuInfoMap["size"]

        if (mapSize === 0)
            VideoDeviceContextMenuItemCreation.createVideoDeviceContextMenuItemObjects(
                        JamiStrings.noVideoDevice, false)

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

        onAccepted: AvAdapter.shareFile(jamiFileDialog.file)
    }

    Component.onCompleted: {
        ContextMenuGenerator.createBaseContextMenuObjects(root)
        VideoDeviceContextMenuItemCreation.setVideoContextMenuObject(ContextMenuGenerator.getMenu())

        ContextMenuGenerator.getMenu().closed.connect(function (){
            VideoDeviceContextMenuItemCreation.removeCreatedItems()
        })
    }

    // TODO: In the future we want to implement this

    // GeneralMenuItem {
    //     id: advancedInfosItem

    //     itemName: qsTr("Advanced informations")
    //     iconSource: "qrc:/images/icons/info-24px.svg"
    //     leftBorderWidth: commonBorderWidth
    //     rightBorderWidth: commonBorderWidth

    //     onClicked: {
    //         root.close()
    //     }
    // }
}

