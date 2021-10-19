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

import QtQuick 2.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"
import "../../commoncomponents/contextmenu"
import "../js/selectscreenwindowcreation.js" as SelectScreenWindowCreation
import "../js/screenrubberbandcreation.js" as ScreenRubberBandCreation

ContextMenuAutoLoader {
    id: root

    property bool isSIP: false
    property bool isPaused: false
    property bool isAudioOnly: false
    property bool localIsRecording: false
    property bool peerIsRecording: false

    signal pluginItemClicked
    signal transferCallButtonClicked

    property list<GeneralMenuItem> menuItems: [
        GeneralMenuItem {
            id: resumePauseCall

            canTrigger: isSIP
            itemName: isPaused ? JamiStrings.resumeCall : JamiStrings.pauseCall
            iconSource: isPaused ?
                            JamiResources.play_circle_outline_24dp_svg :
                            JamiResources.pause_circle_outline_24dp_svg
            onClicked: {
                CallAdapter.holdThisCallToggle()
            }
        },
        GeneralMenuItem {
            id: inputPanelSIP

            canTrigger: isSIP
            itemName: JamiStrings.sipInputPanel
            iconSource: JamiResources.ic_keypad_svg
            onClicked: {
                sipInputPanel.open()
            }
        },
        GeneralMenuItem {
            id: callTransfer

            canTrigger: isSIP
            itemName: JamiStrings.transferCall
            iconSource: JamiResources.phone_forwarded_24dp_svg
            addMenuSeparatorAfter: isSIP
            onClicked: {
                root.transferCallButtonClicked()
            }
        },
        GeneralMenuItem {
            id: localRecord

            itemName: localIsRecording ? JamiStrings.stopRec : JamiStrings.startRec
            iconSource: JamiResources.fiber_manual_record_24dp_svg
            iconColor: JamiTheme.recordIconColor
            onClicked: {
                CallAdapter.recordThisCallToggle()
                localIsRecording = CallAdapter.isRecordingThisCall()
            }
        },
        GeneralMenuItem {
            id: fullScreen

            itemName: JamiQmlUtils.callIsFullscreen ?
                          JamiStrings.exitFullScreen : JamiStrings.fullScreen
            iconSource: JamiQmlUtils.callIsFullscreen ?
                            JamiResources.close_fullscreen_24dp_svg :
                            JamiResources.open_in_full_24dp_svg
            onClicked: {
                callStackView.toggleFullScreen()
            }
        },
        GeneralMenuItem {
            id: stopSharing

            canTrigger: AvAdapter.currentRenderingDeviceType === Video.DeviceType.DISPLAY
                        && !isSIP
            itemName: JamiStrings.stopSharing
            iconSource: JamiResources.share_stop_black_24dp_svg
            iconColor: JamiTheme.redColor
            onClicked: {
                AvAdapter.stopSharing()
            }
        },
        GeneralMenuItem {
            id: shareScreen

            canTrigger: AvAdapter.currentRenderingDeviceType !== Video.DeviceType.DISPLAY
                        && !isSIP
            itemName: JamiStrings.shareScreen
            iconSource: JamiResources.share_screen_black_24dp_svg
            onClicked: {
                if (Qt.application.screens.length === 1) {
                    AvAdapter.shareEntireScreen(0)
                } else {
                    SelectScreenWindowCreation.createSelectScreenWindowObject()
                    SelectScreenWindowCreation.showSelectScreenWindow()
                }
            }
        },
        GeneralMenuItem {
            id: shareScreenArea

            canTrigger: AvAdapter.currentRenderingDeviceType !== Video.DeviceType.DISPLAY
                        && !isSIP
            itemName: JamiStrings.shareScreenArea
            iconSource: JamiResources.share_screen_black_24dp_svg
            onClicked: {
                if (Qt.platform.os !== "windows") {
                    AvAdapter.shareScreenArea(0, 0, 0, 0)
                } else {
                    ScreenRubberBandCreation.createScreenRubberBandWindowObject()
                    ScreenRubberBandCreation.showScreenRubberBandWindow()
                }
            }
        },
        GeneralMenuItem {
            id: shareFile

            canTrigger: !isSIP
            itemName: JamiStrings.shareFile
            iconSource: JamiResources.insert_photo_24dp_svg
            onClicked: {
                jamiFileDialog.open()
            }
        },
        GeneralMenuItem {
            id: viewPlugin

            canTrigger: PluginAdapter.callMediaHandlersListCount
            itemName: JamiStrings.viewPlugin
            iconSource: JamiResources.extension_24dp_svg
            onClicked: {
                root.pluginItemClicked()
            }
        }
    ]

    Component.onCompleted: menuItemsToLoad = menuItems
}
