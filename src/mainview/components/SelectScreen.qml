
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
import QtQuick.Controls.Universal 2.12
import net.jami.Models 1.0

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


    // Decide whether to show screen area or entire screen.
    property bool selectArea: false


    // How many rows the ScrollView should have.
    function calculateRepeaterModel() {
        var numberOfScreens = Qt.application.screens.length

        if (numberOfScreens % 2 === 1)
            return numberOfScreens / 2 + 1
        else
            return numberOfScreens / 2
    }

    function calculateScreenNumber(index) {
        if (index === 0 || index === 1)
            return index
        if (index % 2 === 0)
            return index * 2
        else
            return index * 2 + 1
    }

    minimumWidth: minWidth
    minimumHeight: minHeight

    title: "Screen sharing"


    // Note: Qt.application.screens[0] is the app's current existing screen.
    screen: Qt.application.screens[0]

    Rectangle {
        id: selectScreenWindowRect

        anchors.fill: parent

        Text {
            id: screenListText

            anchors.top: selectScreenWindowRect.top
            anchors.topMargin: 20
            anchors.horizontalCenter: selectScreenWindowRect.horizontalCenter

            font.pointSize: JamiTheme.textFontSize + 2
            font.bold: true
            text: qsTr("Choose A Screen to Share")
        }

        ScrollView {
            id: screenSelectionScrollView

            anchors.centerIn: selectScreenWindowRect

            width: selectScreenWindowRect.width - 50
            height: selectScreenWindowRect.height - 150

            clip: true

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff


            // Column of rows repeater (two screen captures in a row).
            Column {
                id: screenSelectionScrollViewColumn

                spacing: 10

                Repeater {
                    id: screenInfo

                    model: calculateRepeaterModel()

                    Row {
                        id: screenInfoRow

                        spacing: 20

                        Connections {
                            target: selectScreenWindow

                            function onSelectedScreenNumberChanged() {


                                // Recover from green state.
                                screenSelectionRectOdd.borderColor = JamiTheme.tabbarBorderColor
                                screenSelectionRectEven.borderColor = JamiTheme.tabbarBorderColor
                            }
                        }


                        // To make sure that two screen captures in one row,
                        // a repeater of two rect is needed, which one in charge
                        // of odd number screen, one in charge of even number screen.
                        Rectangle {
                            id: screenSelectionRectOdd

                            property string borderColor: JamiTheme.tabbarBorderColor

                            height: screenSelectionScrollView.height
                            width: screenSelectionScrollView.width / 2 - screenInfoRow.spacing / 2

                            radius: 10
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

                                Component.onCompleted: {
                                    screenShotOdd.source = "data:image/png;base64,"
                                            + AvAdapter.captureScreen(
                                                calculateScreenNumber(index))
                                }
                            }

                            Text {
                                id: screenNameOdd

                                anchors.top: screenShotOdd.bottom
                                anchors.topMargin: 10
                                anchors.horizontalCenter: screenSelectionRectOdd.horizontalCenter

                                font.pointSize: JamiTheme.textFontSize - 2
                                text: qsTr("Screen") + " " + (calculateScreenNumber(
                                                                  index) + 1)
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton

                                onClicked: {
                                    if (selectedScreenNumber == -1
                                            || selectedScreenNumber !== calculateScreenNumber(
                                                index)) {
                                        selectedScreenNumber = calculateScreenNumber(
                                                    index)
                                        screenSelectionRectOdd.borderColor
                                                = JamiTheme.screenSelectionBorderGreen
                                    }
                                }
                            }
                        }

                        Rectangle {
                            id: screenSelectionRectEven

                            property string borderColor: JamiTheme.tabbarBorderColor

                            height: screenSelectionScrollView.height
                            width: screenSelectionScrollView.width / 2 - screenInfoRow.spacing / 2

                            radius: 10
                            border.color: borderColor

                            visible: (Qt.application.screens.length) % 2 != 1

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
                                        screenShotEven.source = "data:image/png;base64,"
                                                + AvAdapter.captureScreen(
                                                    calculateScreenNumber(
                                                        index + 1))
                                }
                            }

                            Text {
                                id: screenNameEven

                                anchors.top: screenShotEven.bottom
                                anchors.topMargin: 10
                                anchors.horizontalCenter: screenSelectionRectEven.horizontalCenter

                                font.pointSize: JamiTheme.textFontSize - 2
                                text: qsTr(
                                          "Screen") + " " + (calculateScreenNumber(
                                                                 index + 1) + 1)
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton

                                onClicked: {
                                    if (selectedScreenNumber == -1
                                            || selectedScreenNumber !== calculateScreenNumber(
                                                index + 1)) {
                                        selectedScreenNumber = calculateScreenNumber(
                                                    index + 1)
                                        screenSelectionRectEven.borderColor
                                                = JamiTheme.screenSelectionBorderGreen
                                    }
                                }
                            }
                        }
                    }
                }
            }

            background: Rectangle {
                id: screenSelectionScrollViewBackground

                radius: 10
                border.color: JamiTheme.tabbarBorderColor
            }
        }
    }

    HoverableButton {
        id: selectButton

        anchors.bottom: selectScreenWindowRect.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: selectScreenWindowRect.horizontalCenter

        visible: selectedScreenNumber != -1

        text: qsTr("Share Screen")
        radius: 10

        onClicked: {
            if (selectArea) {
                selectScreenWindow.hide()
                ScreenRubberBandCreation.createScreenRubberBandWindowObject(
                            selectScreenWindow, selectedScreenNumber)
                ScreenRubberBandCreation.showScreenRubberBandWindow()


                // Destory selectScreenWindow once screenRubberBand is closed.
                ScreenRubberBandCreation.connectOnClosingEvent(function () {
                    selectScreenWindow.close()
                })
            } else {
                AvAdapter.shareEntireScreen(selectedScreenNumber)
                selectScreenWindow.close()
            }
        }
    }
}
