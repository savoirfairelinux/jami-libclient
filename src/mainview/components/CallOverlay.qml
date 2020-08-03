
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
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.12
import QtQml 2.14
import net.jami.Models 1.0

import "../js/contactpickercreation.js" as ContactPickerCreation

import "../../commoncomponents"

Rectangle {
    id: callOverlayRect

    property string bestName: "Best Name"
    property string timeText: "00:00"

    signal backButtonIsClicked
    signal overlayChatButtonClicked

    function updateButtonStatus(isPaused, isAudioOnly, isAudioMuted, isVideoMuted, isRecording, isSIP, isConferenceCall) {
        callOverlayButtonGroup.setButtonStatus(isPaused, isAudioOnly,
                                               isAudioMuted, isVideoMuted,
                                               isRecording, isSIP,
                                               isConferenceCall)
    }

    function showOnHoldImage(visible) {
        onHoldImage.visible = visible
    }

    function closePotentialContactPicker() {
        ContactPickerCreation.closeContactPicker()
    }

    function setBackTintedButtonVisible(visible) {
        backTintedButton.visible = visible
    }

    anchors.fill: parent


    /*
     * Timer to decide when overlay fade out.
     */
    Timer {
        id: callOverlayTimer
        interval: 5000
        onTriggered: {
            if (overlayUpperPartRect.state !== 'freezed') {
                overlayUpperPartRect.state = 'freezed'
            }
            if (callOverlayButtonGroup.state !== 'freezed') {
                callOverlayButtonGroup.state = 'freezed'
            }
        }
    }

    Rectangle {
        id: overlayUpperPartRect

        anchors.top: callOverlayRect.top

        width: callOverlayRect.width
        height: 50
        opacity: 0

        RowLayout {
            id: overlayUpperPartRectRowLayout

            anchors.fill: parent

            TintedButton {
                id: backTintedButton

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.leftMargin: 5
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30

                tintColor: JamiTheme.buttonTintedBlue
                normalPixmapSource: "qrc:/images/icons/ic_arrow_back_white_24dp.png"
                selectedPixmapSource: "qrc:/images/icons/ic_arrow_back_white_24dp.png"

                onClicked: {
                    callOverlayRect.backButtonIsClicked()
                }

                onButtonEntered: {
                    callOverlayRectMouseArea.entered()
                }
            }

            Text {
                id: jamiBestNameText

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: overlayUpperPartRect.width / 3
                Layout.preferredHeight: 50

                font.pointSize: JamiTheme.textFontSize

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiBestNameText.elidedText
                color: "white"

                TextMetrics {
                    id: textMetricsjamiBestNameText
                    font: jamiBestNameText.font
                    text: bestName
                    elideWidth: overlayUpperPartRect.width / 3
                    elide: Qt.ElideMiddle
                }
            }

            Text {
                id: callTimerText

                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.preferredWidth: overlayUpperPartRect.width / 3
                Layout.preferredHeight: 50

                font.pointSize: JamiTheme.textFontSize

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricscallTimerText.elidedText
                color: "white"

                TextMetrics {
                    id: textMetricscallTimerText
                    font: callTimerText.font
                    text: timeText
                    elideWidth: overlayUpperPartRect.width / 3
                    elide: Qt.ElideMiddle
                }
            }
        }

        color: "transparent"


        /*
         * Rect states: "entered" state should make overlay fade in,
         *              "freezed" state should make overlay fade out.
         * Combine with PropertyAnimation of opacity.
         */
        states: [
            State {
                name: "entered"
                PropertyChanges {
                    target: overlayUpperPartRect
                    opacity: 1
                }
            },
            State {
                name: "freezed"
                PropertyChanges {
                    target: overlayUpperPartRect
                    opacity: 0
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation {
                target: overlayUpperPartRect
                property: "opacity"
                duration: 1000
            }
        }
    }

    Image {
        id: onHoldImage

        anchors.verticalCenter: callOverlayRect.verticalCenter
        anchors.horizontalCenter: callOverlayRect.horizontalCenter

        width: 200
        height: 200

        visible: false

        fillMode: Image.PreserveAspectFit
        source: "qrc:/images/icons/ic_pause_white_100px.png"
        asynchronous: true
    }

    CallOverlayButtonGroup {
        id: callOverlayButtonGroup

        anchors.bottom: callOverlayRect.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: callOverlayRect.horizontalCenter

        width: callOverlayRect.width / 3 * 2
        height: 60
        opacity: 0

        onChatButtonClicked: {
            callOverlayRect.overlayChatButtonClicked()
        }

        onAddToConferenceButtonClicked: {


            /*
             * Create contact picker - conference.
             */
            ContactPickerCreation.createContactPickerObjects(
                        ContactPicker.ContactPickerType.JAMICONFERENCE,
                        callOverlayRect)
            ContactPickerCreation.calculateCurrentGeo(
                        callOverlayRect.width / 2, callOverlayRect.height / 2)
            ContactPickerCreation.openContactPicker()
        }

        onTransferCallButtonClicked: {


            /*
             * Create contact picker - sip transfer.
             */
            ContactPickerCreation.createContactPickerObjects(
                        ContactPicker.ContactPickerType.SIPTRANSFER,
                        callOverlayRect)
            ContactPickerCreation.calculateCurrentGeo(
                        callOverlayRect.width / 2, callOverlayRect.height / 2)
            ContactPickerCreation.openContactPicker()
        }

        onButtonEntered: {
            callOverlayRectMouseArea.entered()
        }

        states: [
            State {
                name: "entered"
                PropertyChanges {
                    target: callOverlayButtonGroup
                    opacity: 1
                }
            },
            State {
                name: "freezed"
                PropertyChanges {
                    target: callOverlayButtonGroup
                    opacity: 0
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation {
                target: callOverlayButtonGroup
                property: "opacity"
                duration: 1000
            }
        }
    }


    /*
     * MouseAreas to make sure that overlay states are correctly set.
     */
    MouseArea {
        id: callOverlayButtonGroupLeftSideMouseArea

        anchors.bottom: callOverlayRect.bottom
        anchors.left: callOverlayRect.left

        width: callOverlayRect.width / 6
        height: 60

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            callOverlayRectMouseArea.entered()
        }

        onMouseXChanged: {
            callOverlayRectMouseArea.entered()
        }
    }

    MouseArea {
        id: callOverlayButtonGroupRightSideMouseArea

        anchors.bottom: callOverlayRect.bottom
        anchors.right: callOverlayRect.right

        width: callOverlayRect.width / 6
        height: 60

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            callOverlayRectMouseArea.entered()
        }

        onMouseXChanged: {
            callOverlayRectMouseArea.entered()
        }
    }

    MouseArea {
        id: callOverlayRectMouseArea

        anchors.top: callOverlayRect.top
        anchors.topMargin: 50

        width: callOverlayRect.width
        height: callOverlayRect.height - callOverlayButtonGroup.height - 50

        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            if (overlayUpperPartRect.state !== 'entered') {
                overlayUpperPartRect.state = 'entered'
            }
            if (callOverlayButtonGroup.state !== 'entered') {
                callOverlayButtonGroup.state = 'entered'
            }
            callOverlayTimer.restart()
        }

        onMouseXChanged: {
            if (overlayUpperPartRect.state !== 'entered') {
                overlayUpperPartRect.state = 'entered'
            }
            if (callOverlayButtonGroup.state !== 'entered') {
                callOverlayButtonGroup.state = 'entered'
            }
            callOverlayTimer.restart()
        }
    }

    color: "transparent"

    onBestNameChanged: {
        ContactAdapter.setCalleeDisplayName(bestName)
    }

    onWidthChanged: {
        ContactPickerCreation.calculateCurrentGeo(callOverlayRect.width / 2,
                                                  callOverlayRect.height / 2)
    }

    onHeightChanged: {
        ContactPickerCreation.calculateCurrentGeo(callOverlayRect.width / 2,
                                                  callOverlayRect.height / 2)
    }
}
