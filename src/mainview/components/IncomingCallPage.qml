
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

import "../../commoncomponents"


/*
 * IncomingCallPage as a seperate window,
 * exist at the right bottom, as a notification to user that
 * a call is incoming.
 */
Window {
    id: incomingCallPage

    property int minWidth: 300
    property int minHeight: 400


    /*
     * The unique identifier for incomingCallPage
     */
    property string responsibleAccountId: ""
    property string responsibleConvUid: ""

    property string contactImgSource: ""
    property string bestName: "Best Name"
    property string bestId: "Best Id"

    property int buttonPreferredSize: 50
    property variant clickPos: "1,1"

    function updateUI() {
        incomingCallPage.contactImgSource = "data:image/png;base64,"
                + ClientWrapper.utilsAdaptor.getContactImageString(responsibleAccountId,
                                                     responsibleConvUid)
        incomingCallPage.bestName = ClientWrapper.utilsAdaptor.getBestName(
                    responsibleAccountId, responsibleConvUid)
        var id = ClientWrapper.utilsAdaptor.getBestId(responsibleAccountId,
                                        responsibleConvUid)
        incomingCallPage.bestId = (incomingCallPage.bestName !== id) ? id : ""
    }

    function updatePositionToRightBottom() {


        /*
         * Screen right bottom,
         * since the qt screen.virtualY, virtualX does not work properly,
         * we need to calculate the screen x, y ourselves, by
         * using to fact that window will always be in the middle if no x or y
         * specificed.
         * ex: https://doc.qt.io/qt-5/qscreen.html#geometry-prop
         */
        var virtualX = (incomingCallPage.x + width / 2) - screen.width / 2
        incomingCallPage.x = virtualX + screen.width - width
        incomingCallPage.y = screen.height - height - 50
    }

    minimumWidth: minWidth
    minimumHeight: minHeight

    maximumWidth: minWidth + 300
    maximumHeight: minHeight + 300

    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    screen: Qt.application.screens[0]

    Rectangle {
        id: incomingCallPageColumnLayoutMainRect

        anchors.fill: parent

        radius: 15
        color: "black"


        /*
         * Simulate window drag. (top with height 30).
         */
        MouseArea {
            id: dragMouseArea

            anchors.left: incomingCallPageColumnLayoutMainRect.left
            anchors.top: incomingCallPageColumnLayoutMainRect.top

            width: incomingCallPageColumnLayoutMainRect.width - closeButton.width - 10
            height: 30

            onPressed: {
                clickPos = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: {
                var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                incomingCallPage.x += delta.x
                incomingCallPage.y += delta.y
            }
        }

        HoverableButton {
            id: closeButton

            anchors.top: incomingCallPageColumnLayoutMainRect.top
            anchors.topMargin: 10
            anchors.right: incomingCallPageColumnLayoutMainRect.right
            anchors.rightMargin: 10

            width: 30
            height: 30

            radius: 30
            source: "qrc:/images/icons/ic_close_white_24dp.png"
            backgroundColor: "black"
            onEnterColor: JamiTheme.closeButtonLighterBlack
            onExitColor: "black"
            onPressColor: JamiTheme.declineButtonPressedRed
            onReleaseColor: "black"

            onClicked: {
                incomingCallPage.close()
                CallAdapter.refuseACall(responsibleAccountId,
                                        responsibleConvUid)
            }
        }

        ColumnLayout {
            id: incomingCallPageColumnLayout

            anchors.fill: parent

            Image {
                id: contactImg

                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 30

                Layout.preferredWidth: 100
                Layout.preferredHeight: 100

                fillMode: Image.PreserveAspectFit
                source: contactImgSource
            }

            Rectangle {
                id: incomingCallPageTextRect

                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 5

                Layout.preferredWidth: incomingCallPage.width
                Layout.preferredHeight: jamiBestNameText.height + jamiBestIdText.height
                                        + talkToYouText.height + 20

                ColumnLayout {
                    id: incomingCallPageTextRectColumnLayout

                    Text {
                        id: jamiBestNameText

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: incomingCallPageTextRect.width
                        Layout.preferredHeight: 50

                        font.pointSize: JamiTheme.textFontSize + 3

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        text: textMetricsjamiBestNameText.elidedText
                        color: "white"

                        TextMetrics {
                            id: textMetricsjamiBestNameText
                            font: jamiBestNameText.font
                            text: bestName
                            elideWidth: incomingCallPageTextRect.width - 30
                            elide: Qt.ElideMiddle
                        }
                    }

                    Text {
                        id: jamiBestIdText

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: incomingCallPageTextRect.width
                        Layout.preferredHeight: 30

                        font.pointSize: JamiTheme.textFontSize

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        text: textMetricsjamiBestIdText.elidedText
                        color: "white"

                        TextMetrics {
                            id: textMetricsjamiBestIdText
                            font: jamiBestIdText.font
                            text: bestId
                            elideWidth: incomingCallPageTextRect.width - 30
                            elide: Qt.ElideMiddle
                        }
                    }

                    Text {
                        id: talkToYouText

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: incomingCallPageTextRect.width
                        Layout.preferredHeight: 30

                        font.pointSize: JamiTheme.textFontSize

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: "white"

                        text: "is calling you"
                    }
                }

                color: "transparent"
            }

            RowLayout {
                id: incomingCallPageRowLayout

                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.bottomMargin: 5

                Layout.preferredWidth: incomingCallPage.width - 100
                Layout.preferredHeight: buttonPreferredSize

                ColumnLayout {
                    id: callAnswerButtonColumnLayout

                    Layout.alignment: Qt.AlignLeft

                    HoverableButton {
                        id: callAnswerButton

                        Layout.alignment: Qt.AlignCenter

                        Layout.preferredWidth: buttonPreferredSize
                        Layout.preferredHeight: buttonPreferredSize

                        backgroundColor: JamiTheme.acceptButtonGreen
                        onEnterColor: JamiTheme.acceptButtonHoverGreen
                        onPressColor: JamiTheme.acceptButtonPressedGreen
                        onReleaseColor: JamiTheme.acceptButtonHoverGreen
                        onExitColor: JamiTheme.acceptButtonGreen

                        buttonImageHeight: buttonPreferredSize / 2
                        buttonImageWidth: buttonPreferredSize / 2
                        source: "qrc:/images/icons/ic_check_white_18dp_2x.png"
                        radius: 30

                        onClicked: {
                            incomingCallPage.close()
                            CallAdapter.acceptACall(responsibleAccountId,
                                                    responsibleConvUid)
                        }
                    }

                    Text {
                        id: answerText

                        Layout.alignment: Qt.AlignCenter

                        font.pointSize: JamiTheme.textFontSize - 2
                        text: qsTr("Answer")
                    }
                }

                ColumnLayout {
                    id: callDeclineButtonColumnLayout

                    Layout.alignment: Qt.AlignRight

                    HoverableButton {
                        id: callDeclineButton

                        Layout.alignment: Qt.AlignCenter

                        Layout.preferredWidth: buttonPreferredSize
                        Layout.preferredHeight: buttonPreferredSize

                        backgroundColor: JamiTheme.declineButtonRed
                        onEnterColor: JamiTheme.declineButtonHoverRed
                        onPressColor: JamiTheme.declineButtonPressedRed
                        onReleaseColor: JamiTheme.declineButtonHoverRed
                        onExitColor: JamiTheme.declineButtonRed

                        buttonImageHeight: buttonPreferredSize / 2
                        buttonImageWidth: buttonPreferredSize / 2
                        source: "qrc:/images/icons/ic_close_white_24dp.png"
                        radius: 30

                        onClicked: {
                            incomingCallPage.close()
                            CallAdapter.refuseACall(responsibleAccountId,
                                                    responsibleConvUid)
                        }
                    }

                    Text {
                        id: ignoreText

                        Layout.alignment: Qt.AlignCenter

                        font.pointSize: JamiTheme.textFontSize - 2
                        text: qsTr("Ignore")
                    }
                }
            }
        }
    }

    color: "transparent"

    Shortcut {
        sequence: "Ctrl+Y"
        context: Qt.ApplicationShortcut
        onActivated: {
            incomingCallPage.close()
            CallAdapter.acceptACall(responsibleAccountId,
                                    responsibleConvUid)
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+D"
        context: Qt.ApplicationShortcut
        onActivated: {
            incomingCallPage.close()
            CallAdapter.refuseACall(responsibleAccountId,
                                    responsibleConvUid)
        }
    }
}
