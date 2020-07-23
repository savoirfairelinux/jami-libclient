
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
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: outgoingCallPageRect

    property int buttonPreferredSize: 50
    property string callStatusPresentation: "outgoing"
    property string contactImgSource: ""
    property string bestName: "Best Name"
    property string bestId: "Best Id"

    signal callCancelButtonIsClicked

    function updateUI(accountId, convUid) {
        contactImgSource = "data:image/png;base64," + ClientWrapper.utilsAdaptor.getContactImageString(
                    accountId, convUid)
        bestName = ClientWrapper.utilsAdaptor.getBestName(accountId, convUid)
        var id = ClientWrapper.utilsAdaptor.getBestId(accountId, convUid)
        bestId = (bestName !== id) ? id : ""
    }

    anchors.fill: parent

    color: "black"


    /*
     * Prevent right click propagate to VideoCallPage.
     */
    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        acceptedButtons: Qt.RightButton
    }

    ColumnLayout {
        id: outgoingCallPageRectColumnLayout

        anchors.fill: parent

        Image {
            id: contactImg

            Layout.alignment: Qt.AlignCenter

            Layout.preferredWidth: 100
            Layout.preferredHeight: 100

            fillMode: Image.PreserveAspectFit
            source: contactImgSource
            asynchronous: true
        }

        Rectangle {
            id: outgoingCallPageTextRect

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 5

            Layout.preferredWidth: outgoingCallPageRect.width
            Layout.preferredHeight: jamiBestNameText.height + jamiBestIdText.height
                                    + callStatusText.height + spinnerImage.height + 20

            color: "transparent"

            ColumnLayout {
                id: outgoingCallPageTextRectColumnLayout

                Text {
                    id: jamiBestNameText

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: outgoingCallPageTextRect.width
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
                        elideWidth: outgoingCallPageTextRect.width - 50
                        elide: Qt.ElideMiddle
                    }
                }

                Text {
                    id: jamiBestIdText

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: outgoingCallPageTextRect.width
                    Layout.preferredHeight: 30

                    font.pointSize: JamiTheme.textFontSize

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    text: textMetricsjamiBestIdText.elidedText
                    color: Qt.lighter("white", 1.5)

                    TextMetrics {
                        id: textMetricsjamiBestIdText
                        font: jamiBestIdText.font
                        text: bestId
                        elideWidth: outgoingCallPageTextRect.width - 50
                        elide: Qt.ElideMiddle
                    }
                }

                AnimatedImage {
                    id: spinnerImage

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 5

                    source: "qrc:/images/waiting.gif"
                }

                Text {
                    id: callStatusText

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: outgoingCallPageTextRect.width
                    Layout.preferredHeight: 30

                    font.pointSize: JamiTheme.textFontSize

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    text: callStatusPresentation + "..."
                    color: Qt.lighter("white", 1.5)
                }
            }
        }

        ColumnLayout {
            id: callCancelButtonColumnLayout

            Layout.alignment: Qt.AlignCenter

            HoverableButton {
                id: callCancelButton

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
                    outgoingCallPageRect.callCancelButtonIsClicked()
                }
            }

            Text {
                id: cancelText

                Layout.alignment: Qt.AlignCenter

                font.pointSize: JamiTheme.textFontSize - 2
                text: qsTr("Cancel")
            }
        }
    }
}
