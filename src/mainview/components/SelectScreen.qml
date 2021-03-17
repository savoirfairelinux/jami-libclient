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

import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../js/screenrubberbandcreation.js" as ScreenRubberBandCreation
import "../../commoncomponents"

// SelectScreenWindow as a seperate window,
// is to make user aware of which screen they want to share,
// during the video call, if the context menu item is selected.
Window {
    id: selectScreenWindow

    property int minWidth: 650
    property int minHeight: 500

    property int selectedScreenNumber: -1
    property bool selectAllScreens: false

    // How many rows the ScrollView should have.
    function calculateRepeaterModel() {
        var numberOfScreens = Qt.application.screens.length

        return Math.ceil(numberOfScreens / 2)
    }

    function calculateScreenNumber(index, isEven) {
        return index * 2 + (isEven ? 2 : 1)
    }

    minimumWidth: minWidth
    minimumHeight: minHeight

    width: minWidth
    height: minHeight

    screen: JamiQmlUtils.mainApplicationScreen

    modality: Qt.ApplicationModal

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

        ScrollView {
            id: screenSelectionScrollView

            anchors.top: screenListText.bottom
            anchors.topMargin: JamiTheme.preferredMarginSize
            anchors.horizontalCenter: selectScreenWindowRect.horizontalCenter

            width: selectScreenWindowRect.width
            height: selectScreenWindowRect.height -
                    (screenListText.height + selectButton.height + JamiTheme.preferredMarginSize * 4)

            clip: true

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            // Column of rows repeater (two screen captures in a row).
            Column {
                id: screenSelectionScrollViewColumn

                spacing: 10

                Repeater {
                    id: screenInfo

                    model: calculateRepeaterModel()

                    Row {
                        id: screenInfoRow

                        leftPadding: JamiTheme.preferredMarginSize
                        rightPadding: JamiTheme.preferredMarginSize
                        spacing: screenSelectionScrollViewColumn.spacing

                        Connections {
                            target: selectScreenWindow

                            function onSelectedScreenNumberChanged() {
                                // Recover from green state.
                                screenSelectionRectOdd.borderColor = JamiTheme.tabbarBorderColor
                                screenSelectionRectEven.borderColor = JamiTheme.tabbarBorderColor
                            }
                        }

                        Connections {
                            target: AvAdapter

                            function onScreenCaptured(screenNumber, source) {
                                if (screenNumber === -1)
                                    screenShotAll.source = JamiQmlUtils.base64StringTitle + source
                                if (screenNumber !== index && screenNumber !== index + 1)
                                    return
                                if (screenNumber % 2 !== 1)
                                    screenShotOdd.source = JamiQmlUtils.base64StringTitle + source
                                else
                                    screenShotEven.source = JamiQmlUtils.base64StringTitle + source
                            }
                        }

                        // To make sure that two screen captures in one row,
                        // a repeater of two rect is needed, which one in charge
                        // of odd number screen, one in charge of even number screen.
                        Rectangle {
                            id: screenSelectionRectOdd

                            property string borderColor: JamiTheme.tabbarBorderColor

                            color: JamiTheme.secondaryBackgroundColor

                            height: screenSelectionScrollView.height
                            width: screenSelectionScrollView.width / 2 -
                                   screenInfoRow.spacing / 2 - JamiTheme.preferredMarginSize

                            border.color: borderColor

                            Image {
                                id: screenShotOdd

                                anchors.top: screenSelectionRectOdd.top
                                anchors.topMargin: 10
                                anchors.horizontalCenter: screenSelectionRectOdd.horizontalCenter

                                height: screenSelectionRectOdd.height - 50
                                width: screenSelectionRectOdd.width - 50

                                fillMode: Image.PreserveAspectFit
                                mipmap: true

                                Component.onCompleted: AvAdapter.captureScreen(
                                                           calculateScreenNumber(index, false) - 1)
                            }

                            Text {
                                id: screenNameOdd

                                anchors.top: screenShotOdd.bottom
                                anchors.topMargin: 10
                                anchors.horizontalCenter: screenSelectionRectOdd.horizontalCenter

                                font.pointSize: JamiTheme.textFontSize - 2
                                text: qsTr("Screen") + " " + calculateScreenNumber(index, false)
                                color: JamiTheme.textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton

                                onClicked: {
                                    if (selectedScreenNumber == -1
                                            || selectedScreenNumber !==
                                            calculateScreenNumber(index, false)) {
                                        selectedScreenNumber = calculateScreenNumber(index, false)
                                        screenSelectionRectOdd.borderColor
                                                = JamiTheme.screenSelectionBorderGreen
                                    }
                                }
                            }
                        }

                        Rectangle {
                            id: screenSelectionRectEven

                            property string borderColor: JamiTheme.tabbarBorderColor

                            color: JamiTheme.secondaryBackgroundColor

                            height: screenSelectionScrollView.height
                            width: screenSelectionScrollView.width / 2 -
                                   screenInfoRow.spacing / 2 - JamiTheme.preferredMarginSize

                            border.color: borderColor

                            visible: {
                                if (calculateScreenNumber(index, true) >=
                                        Qt.application.screens.length)
                                    return (Qt.application.screens.length) % 2 != 1
                                return true
                            }

                            Image {
                                id: screenShotEven

                                anchors.top: screenSelectionRectEven.top
                                anchors.topMargin: 10
                                anchors.horizontalCenter: screenSelectionRectEven.horizontalCenter

                                height: screenSelectionRectEven.height - 50
                                width: screenSelectionRectEven.width - 50

                                fillMode: Image.PreserveAspectFit
                                mipmap: true

                                Component.onCompleted: {
                                    if (screenSelectionRectEven.visible)
                                        AvAdapter.captureScreen(
                                                    calculateScreenNumber(index, true) - 1)
                                }
                            }

                            Text {
                                id: screenNameEven

                                anchors.top: screenShotEven.bottom
                                anchors.topMargin: 10
                                anchors.horizontalCenter: screenSelectionRectEven.horizontalCenter

                                font.pointSize: JamiTheme.textFontSize - 2
                                text: qsTr("Screen") + " " + (calculateScreenNumber(index, true))
                                color: JamiTheme.textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton

                                onClicked: {
                                    if (selectedScreenNumber == -1
                                            || selectedScreenNumber !==
                                            calculateScreenNumber(index, true)) {
                                        selectedScreenNumber = calculateScreenNumber(index, true)
                                        screenSelectionRectEven.borderColor
                                                = JamiTheme.screenSelectionBorderGreen
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: screenSelectionRectAll

                    property string borderColor: JamiTheme.tabbarBorderColor

                    anchors.horizontalCenter: screenSelectionScrollViewColumn.horizontalCenter

                    color: JamiTheme.secondaryBackgroundColor

                    height: screenSelectionScrollView.height
                    width: screenSelectionScrollView.width - 2 * JamiTheme.preferredMarginSize

                    border.color: borderColor

                    Connections {
                        target: selectScreenWindow

                        function onSelectedScreenNumberChanged() {
                            // Recover from green state.
                            selectAllScreens = false
                            screenSelectionRectAll.borderColor = JamiTheme.tabbarBorderColor
                        }
                    }

                    Image {
                        id: screenShotAll

                        anchors.top: screenSelectionRectAll.top
                        anchors.topMargin: 10
                        anchors.horizontalCenter: screenSelectionRectAll.horizontalCenter

                        height: screenSelectionRectAll.height - 50
                        width: screenSelectionRectAll.width - 50

                        fillMode: Image.PreserveAspectFit
                        mipmap: true

                        Component.onCompleted: AvAdapter.captureAllScreens()
                    }

                    Text {
                        id: screenNameAll

                        anchors.top: screenShotAll.bottom
                        anchors.topMargin: 10
                        anchors.horizontalCenter: screenSelectionRectAll.horizontalCenter

                        font.pointSize: JamiTheme.textFontSize - 2
                        text: qsTr("All Screens")
                        color: JamiTheme.textColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton

                        onClicked: {
                            selectedScreenNumber = -1
                            selectAllScreens = true
                            screenSelectionRectAll.borderColor
                                    = JamiTheme.screenSelectionBorderGreen
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

        width: 200
        height: 36

        visible: selectedScreenNumber != -1 || selectAllScreens

        color: JamiTheme.buttonTintedBlack
        hoveredColor: JamiTheme.buttonTintedBlackHovered
        pressedColor: JamiTheme.buttonTintedBlackPressed
        outlined: true
        enabled: true

        text: JamiStrings.shareScreen

        onClicked: {
            if (selectAllScreens)
                AvAdapter.shareAllScreens()
            else
                AvAdapter.shareEntireScreen(selectedScreenNumber - 1)
            selectScreenWindow.close()
        }
    }
}
