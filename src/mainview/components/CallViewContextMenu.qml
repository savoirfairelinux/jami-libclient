/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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

import "../js/videodevicecontextmenuitemcreation.js" as VideoDeviceContextMenuItemCreation
import "../js/selectscreenwindowcreation.js" as SelectScreenWindowCreation

Menu {
    id: root

    property int generalMenuSeparatorCount: 0
    property int commonBorderWidth: 1
    font.pointSize: JamiTheme.textFontSize+3

    property bool isSIP: false
    property bool isPaused: false
    property bool isAudioOnly: false
    property bool isRecording: false

    signal transferCallButtonClicked

    function activate() {
        var deviceContextMenuInfoMap = AvAdapter.populateVideoDeviceContextMenuItem()
        /*
         * Somehow, the map size is undefined, so use this instead.
         */
        var mapSize = deviceContextMenuInfoMap["size"]

        var count = 2
        for (var deviceName in deviceContextMenuInfoMap) {
            if (deviceName === "size" || root.isAudioOnly)
                continue
            if (videoDeviceItem.itemName === "No video device") {
                videoDeviceItem.checkable = true
                videoDeviceItem.itemName = deviceName
                videoDeviceItem.checked = deviceContextMenuInfoMap[deviceName]
                if (count === mapSize)
                    root.open()
            } else {
                VideoDeviceContextMenuItemCreation.createVideoDeviceContextMenuItemObjects(
                            deviceName, deviceContextMenuInfoMap[deviceName],
                            count === mapSize)
            }
            count++
        }
        root.open()
    }

    Component.onCompleted: {
        VideoDeviceContextMenuItemCreation.setVideoContextMenuObject(root)
    }


    onClosed: {
        videoDeviceItem.itemName = "No video device"
        VideoDeviceContextMenuItemCreation.removeCreatedItems()
    }

    JamiFileDialog {
        id: jamiFileDialog

        mode: JamiFileDialog.Mode.OpenFile

        onAccepted: {
            // No need to trim file:///.
            AvAdapter.shareFile(jamiFileDialog.file)
        }
    }

    /*
     * All GeneralMenuItems should remain the same width / height.
     */
    GeneralMenuItem {
        id: holdCallButton

        visible: isSIP
        height: isSIP? undefined : 0

        itemName: isPaused? qsTr("Resume call") : qsTr("Hold call")
        iconSource: isPaused? "qrc:/images/icons/play_circle_outline-24px.svg" : "qrc:/images/icons/pause_circle_outline-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            CallAdapter.holdThisCallToggle()
            root.close()
        }
    }

    GeneralMenuItem {
        id: transferCallButton

        visible: isSIP
        height: isSIP? undefined : 0

        itemName: qsTr("Transfer call")
        iconSource: "qrc:/images/icons/phone_forwarded-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            root.transferCallButtonClicked()
            root.close()
        }
    }

    GeneralMenuSeparator {
        preferredWidth: startRecordingItem.preferredWidth
        preferredHeight: commonBorderWidth

        visible: isSIP
        height: isSIP? undefined : 0

        Component.onCompleted: {
            generalMenuSeparatorCount++
        }
    }

    GeneralMenuItem {
        id: startRecordingItem

        itemName: isRecording? qsTr("Stop recording") : qsTr("Start recording")
        iconSource: "qrc:/images/icons/ic_video_call_24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            root.close()
            CallAdapter.recordThisCallToggle()
        }
    }

    GeneralMenuItem {
        id: fullScreenItem

        itemName: videoCallPage.isFullscreen ? qsTr("Exit full screen") : qsTr(
                                     "Full screen mode")
        iconSource: videoCallPage.isFullscreen ? "qrc:/images/icons/close_fullscreen-24px.svg" : "qrc:/images/icons/open_in_full-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            root.close()
            videoCallPageRect.needToShowInFullScreen()
        }
    }

    GeneralMenuSeparator {
        preferredWidth: startRecordingItem.preferredWidth
        preferredHeight: commonBorderWidth

        Component.onCompleted: {
            generalMenuSeparatorCount++
        }
    }

    VideoCallPageContextMenuDeviceItem {
        id: videoDeviceItem
        visible: !isAudioOnly
        height: !isAudioOnly? undefined : 0

        contextMenuPreferredWidth: root.implicitWidth
    }

    GeneralMenuSeparator {
        preferredWidth: startRecordingItem.preferredWidth
        preferredHeight: commonBorderWidth
        visible: !isAudioOnly
        height: !isAudioOnly? undefined : 0

        Component.onCompleted: {
            generalMenuSeparatorCount++
        }
    }

    GeneralMenuItem {
        id: shareEntireScreenItem

        itemName: qsTr("Share entire screen")
        iconSource: "qrc:/images/icons/screen_share-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth
        visible: !isAudioOnly
        height: !isAudioOnly? undefined : 0

        onClicked: {
            root.close()
            if (Qt.application.screens.length === 1) {
                AvAdapter.shareEntireScreen(0)
            } else {
                SelectScreenWindowCreation.createSelectScreenWindowObject()
                SelectScreenWindowCreation.showSelectScreenWindow()
            }
        }
    }

    GeneralMenuItem {
        id: shareScreenAreaItem

        itemName: qsTr("Share screen area")
        iconSource: "qrc:/images/icons/screen_share-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth
        visible: !isAudioOnly
        height: !isAudioOnly? undefined : 0

        onClicked: {
            root.close()
            if (Qt.application.screens.length === 1) {
                ScreenRubberBandCreation.createScreenRubberBandWindowObject(
                            null, 0)
                ScreenRubberBandCreation.showScreenRubberBandWindow()
            } else {
                SelectScreenWindowCreation.createSelectScreenWindowObject(true)
                SelectScreenWindowCreation.showSelectScreenWindow()
            }
        }
    }

    GeneralMenuItem {
        id: shareFileItem

        itemName: qsTr("Share file")
        iconSource: "qrc:/images/icons/insert_photo-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth
        visible: !isAudioOnly
        height: !isAudioOnly? undefined : 0

        onClicked: {
            root.close()
            jamiFileDialog.open()
        }
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
    }

    GeneralMenuItem {
        id: pluginItem

        itemName: qsTr("Toggle plugin")
        iconSource: "qrc:/images/icons/extension_24dp.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            root.close()
        }
    }*/

    background: Rectangle {
        implicitWidth: startRecordingItem.preferredWidth
        implicitHeight: startRecordingItem.preferredHeight
                        * (root.count
                          - (isSIP? 0 : 2)
                          - (isAudioOnly? 6 : 0)
                          - generalMenuSeparatorCount)

        border.width: commonBorderWidth
        border.color: JamiTheme.tabbarBorderColor
    }
}

