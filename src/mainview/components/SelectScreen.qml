/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 *         Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

import QtQuick
import QtQuick.Window
import QtQuick.Controls

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../js/screenrubberbandcreation.js" as ScreenRubberBandCreation
import "../../commoncomponents"

// SelectScreenWindow as a seperate window,
// is to make user aware of which screen they want to share,
// during the video call, if the context menu item is selected.
Window {
    id: root

    property int minWidth: 650
    property int minHeight: 500

    property int selectedScreenNumber: -1
    property bool selectAllScreens: false
    property string currentPreview: ""
    property var screens: []

    // How many rows the ScrollView should have.
    function calculateRepeaterModel() {
        screens = []
        for (var idx in Qt.application.screens) {
            screens.push(qsTr("Screen") + " " + idx)
        }
        AvAdapter.getListWindows()
        for (var idx in AvAdapter.windowsNames) {
            screens.push(AvAdapter.windowsNames[idx])
        } 

        return screens.length
    }

    onActiveChanged: {
        if (!active) {
            selectedScreenNumber = -1
            selectAllScreens = false
        }
        screenInfo.model = {}
        screenInfo2.model = {}
        calculateRepeaterModel()
        screenInfo.model = screens.length
        screenInfo2.model = screens.length
        windowsText.visible = screens.length > Qt.application.screens.length
    }
    minimumWidth: minWidth
    minimumHeight: minHeight

    width: minWidth
    height: minHeight

    screen: JamiQmlUtils.mainApplicationScreen

    Rectangle {
        id: selectScreenWindowRect

        anchors.fill: parent

        color: JamiTheme.backgroundColor

        Text {
            id: screenListText

            anchors.top: selectScreenWindowRect.top
            anchors.topMargin: JamiTheme.preferredMarginSize
            anchors.horizontalCenter: selectScreenWindowRect.horizontalCenter

            font.pointSize: JamiTheme.textFontSize + 2
            font.bold: true
            text: JamiStrings.selectScreen
            color: JamiTheme.textColor
        }

        JamiFlickable {
            id: screenSelectionScrollView

            anchors.topMargin: JamiTheme.preferredMarginSize
            anchors.horizontalCenter: selectScreenWindowRect.horizontalCenter

            width: selectScreenWindowRect.width
            height: selectScreenWindowRect.height -
                    (selectButton.height + JamiTheme.preferredMarginSize * 4)

            contentHeight: screenSelectionScrollViewColumn.implicitHeight

            Flow {
                id: screenSelectionScrollViewFlow

                anchors.fill: parent
                topPadding: JamiTheme.preferredMarginSize
                rightPadding: JamiTheme.preferredMarginSize
                leftPadding: JamiTheme.preferredMarginSize

                spacing: 10

                Text {
                    width: screenSelectionScrollView.width
                    height: JamiTheme.preferredFieldHeight

                    font.pointSize: JamiTheme.menuFontSize
                    font.bold: true
                    text: JamiStrings.screens
                    verticalAlignment: Text.AlignBottom
                    color: JamiTheme.textColor
                }

                Repeater {
                    id: screenInfo

                    model: screens ? screens.length : 0

                    delegate: Rectangle {
                        id: screenItem

                        color: JamiTheme.secondaryBackgroundColor

                        width: screenSelectionScrollView.width / 2 -
                                screenSelectionScrollViewFlow.spacing / 2 - JamiTheme.preferredMarginSize
                        height: 3 * width / 4

                        border.color: selectedScreenNumber === index ? JamiTheme.screenSelectionBorderColor : JamiTheme.tabbarBorderColor
                        visible: JamiStrings.selectScreen !== screens[index] && index < Qt.application.screens.length

                        Text {
                            id: screenName

                            anchors.top: screenItem.top
                            anchors.topMargin: 10
                            anchors.horizontalCenter: screenItem.horizontalCenter
                            width: parent.width
                            font.pointSize: JamiTheme.textFontSize
                            text: screens[index] ? screens[index] : ""
                            elide: Text.ElideMiddle
                            horizontalAlignment: Text.AlignHCenter
                            color: JamiTheme.textColor
                        }

                        PreviewRenderer {
                            id: screenPreview

                            anchors.top: screenName.bottom
                            anchors.topMargin: 10
                            anchors.horizontalCenter: screenItem.horizontalCenter
                            height: screenItem.height - 50
                            width: screenItem.width - 50

                            lrcInstance: LRCInstance

                            Component.onDestruction: {
                                if (screenPreview.rendererId !== "" && screenPreview.rendererId !== currentPreview)
                                    VideoDevices.stopDevice(screenPreview.rendererId, true)
                            }
                            Component.onCompleted: {
                                if (visible) {
                                    var rendId = AvAdapter.getSharingResource(index, "")
                                    if (rendId !== "")
                                        screenPreview.rendererId = VideoDevices.startDevice(rendId, true)
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: screenItem
                            acceptedButtons: Qt.LeftButton

                            onClicked: {
                                selectAllScreens = false
                                if (selectedScreenNumber == -1
                                        || selectedScreenNumber !== index) {
                                    selectedScreenNumber = index
                                }
                            }
                        }

                        Connections {
                            target: AvAdapter

                            function onScreenCaptured(screenNumber, source) {
                                if (screenNumber === -1)
                                    screenShotAll.source = JamiQmlUtils.base64StringTitle + source
                            }
                        }
                    }
                }

                Rectangle {
                    id: screenSelectionRectAll

                    color: JamiTheme.secondaryBackgroundColor

                    width: screenSelectionScrollView.width / 2 -
                                screenSelectionScrollViewFlow.spacing / 2 - JamiTheme.preferredMarginSize
                    height: 3 * width / 4

                    border.color: selectAllScreens ? JamiTheme.screenSelectionBorderColor : JamiTheme.tabbarBorderColor

                    visible: Qt.application.screens.length > 1

                    Text {
                        id: screenNameAll

                        anchors.top: screenSelectionRectAll.top
                        anchors.topMargin: 10
                        anchors.horizontalCenter: screenSelectionRectAll.horizontalCenter

                        font.pointSize: JamiTheme.textFontSize
                        text: JamiStrings.allScreens
                        color: JamiTheme.textColor
                    }

                    PreviewRenderer {
                        id: screenShotAll

                        anchors.top: screenNameAll.bottom
                        anchors.topMargin: 10
                        anchors.horizontalCenter: screenSelectionRectAll.horizontalCenter
                        height: screenSelectionRectAll.height - 50
                        width: screenSelectionRectAll.width - 50

                        lrcInstance: LRCInstance

                        Component.onDestruction: {
                            if (screenShotAll.rendererId !== "" && screenShotAll.rendererId !== currentPreview)
                                VideoDevices.stopDevice(screenShotAll.rendererId, true)
                        }
                        Component.onCompleted: {
                            if (visible) {
                                var rendId = AvAdapter.getSharingResource(-1, "")
                                if (rendId !== "")
                                    screenShotAll.rendererId = VideoDevices.startDevice(rendId, true)
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton

                        onClicked: {
                            selectedScreenNumber = -1
                            selectAllScreens = true
                        }
                    }
                }

                Text {
                    id: windowsText
                    width: screenSelectionScrollView.width
                    height: JamiTheme.preferredFieldHeight

                    font.pointSize: JamiTheme.menuFontSize
                    font.bold: true
                    text: JamiStrings.windows
                    verticalAlignment: Text.AlignBottom
                    color: JamiTheme.textColor
                }

                Repeater {
                    id: screenInfo2

                    model: screens ? screens.length : 0

                    delegate: Rectangle {
                        id: screenItem2

                        color: JamiTheme.secondaryBackgroundColor

                        width: screenSelectionScrollView.width / 2 -
                                screenSelectionScrollViewFlow.spacing / 2 - JamiTheme.preferredMarginSize
                        height: 3 * width / 4

                        border.color: selectedScreenNumber === index ? JamiTheme.screenSelectionBorderColor : JamiTheme.tabbarBorderColor
                        visible: JamiStrings.selectScreen !== screens[index] && index >= Qt.application.screens.length

                        Text {
                            id: screenName2

                            anchors.top: screenItem2.top
                            anchors.topMargin: 10
                            anchors.horizontalCenter: screenItem2.horizontalCenter
                            width: parent.width
                            font.pointSize: JamiTheme.textFontSize
                            text: screens[index] ? screens[index] : ""
                            elide: Text.ElideMiddle
                            horizontalAlignment: Text.AlignHCenter
                            color: JamiTheme.textColor
                        }

                        PreviewRenderer {
                            id: screenPreview2

                            anchors.top: screenName2.bottom
                            anchors.topMargin: 10
                            anchors.horizontalCenter: screenItem2.horizontalCenter
                            anchors.leftMargin: 25
                            anchors.rightMargin: 25
                            height: screenItem2.height - 60
                            width: screenItem2.width - 50

                            lrcInstance: LRCInstance

                            Component.onDestruction: {
                                if (screenPreview2.rendererId !== "" && screenPreview2.rendererId !== currentPreview)
                                    VideoDevices.stopDevice(screenPreview2.rendererId, true)
                            }
                            Component.onCompleted: {
                                if (visible) {
                                    var rendId = AvAdapter.getSharingResource(-2, AvAdapter.windowsIds[index - Qt.application.screens.length])
                                    if (rendId !== "")
                                        screenPreview2.rendererId = VideoDevices.startDevice(rendId, true)
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: screenItem2
                            acceptedButtons: Qt.LeftButton

                            onClicked: {
                                selectAllScreens = false
                                if (selectedScreenNumber == -1
                                        || selectedScreenNumber !== index) {
                                    selectedScreenNumber = index
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    MaterialButton {
        id: selectButton

        anchors.bottom: selectScreenWindowRect.bottom
        anchors.bottomMargin: JamiTheme.preferredMarginSize
        anchors.horizontalCenter: selectScreenWindowRect.horizontalCenter

        preferredWidth: 200

        enabled: selectedScreenNumber != -1 || selectAllScreens
        opacity: enabled ? 1.0 : 0.5

        color: JamiTheme.buttonTintedBlack
        hoveredColor: JamiTheme.buttonTintedBlackHovered
        pressedColor: JamiTheme.buttonTintedBlackPressed
        outlined: true

        text: JamiStrings.shareScreen

        onClicked: {
            if (selectAllScreens)
                AvAdapter.shareAllScreens()
            else {
                if (selectedScreenNumber < Qt.application.screens.length)
                    AvAdapter.shareEntireScreen(selectedScreenNumber)
                else {
                    AvAdapter.shareWindow(AvAdapter.windowsIds[selectedScreenNumber - Qt.application.screens.length])
                }
            }
            root.close()
        }
    }
}
