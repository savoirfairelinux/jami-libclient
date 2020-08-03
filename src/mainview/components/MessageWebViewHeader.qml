
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
import net.jami.Models 1.0

import "../../commoncomponents"

Rectangle {
    id: messagingHeaderRect

    property int buttonPreferredSize: 30
    property string userAliasLabelText: ""
    property string userUserNameLabelText: ""
    property string backToWelcomeViewButtonSource: "qrc:/images/icons/ic_arrow_back_24px.svg"
    property bool sendContactRequestButtonVisible: true

    signal backToWelcomeViewButtonClicked
    signal needToHideConversationInCall
    signal sendContactRequestButtonClicked

    function resetBackToWelcomeViewButtonSource(reset) {
        backToWelcomeViewButtonSource = reset ? "qrc:/images/icons/ic_arrow_back_24px.svg" : "qrc:/images/icons/round-close-24px.svg"
    }

    function toggleMessagingHeaderButtonsVisible(visible) {
        startAAudioCallButton.visible = visible
        startAVideoCallButton.visible = visible
    }

    RowLayout {
        id: messagingHeaderRectRowLayout

        anchors.fill: parent

        HoverableButton {
            id: backToWelcomeViewButton

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.leftMargin: 10
            Layout.preferredWidth: buttonPreferredSize
            Layout.preferredHeight: buttonPreferredSize

            radius: 30
            source: backToWelcomeViewButtonSource
            backgroundColor: "white"
            onExitColor: "white"

            onClicked: {
                if (backToWelcomeViewButtonSource === "qrc:/images/icons/ic_arrow_back_24px.svg")
                    messagingHeaderRect.backToWelcomeViewButtonClicked()
                else
                    messagingHeaderRect.needToHideConversationInCall()
            }
        }

        Rectangle {
            id: userNameOrIdRect

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft


            /*
             * Width + margin.
             */
            Layout.preferredWidth: messagingHeaderRect.width
                                   - backToWelcomeViewButton.width - buttonGroup.width - 45
            Layout.preferredHeight: messagingHeaderRect.height
            Layout.leftMargin: 10

            color: "transparent"

            ColumnLayout {
                id: userNameOrIdColumnLayout

                Label {
                    id: userAliasLabel

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: userNameOrIdRect.width
                    Layout.preferredHeight: textMetricsuserAliasLabel.boundingRect.height
                    Layout.topMargin: userNameOrIdRect.height / 2 - userAliasLabel.height - 4

                    font.pointSize: JamiTheme.textFontSize - 1

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    text: textMetricsuserAliasLabel.elidedText
                }

                TextMetrics {
                    id: textMetricsuserAliasLabel

                    font: userAliasLabel.font
                    text: userAliasLabelText
                    elideWidth: userNameOrIdRect.width
                    elide: Qt.ElideMiddle
                }

                Label {
                    id: userUserNameLabel

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: userNameOrIdRect.width
                    Layout.preferredHeight: textMetricsuserUserNameLabel.boundingRect.height

                    font.pointSize: JamiTheme.textFontSize - 2
                    color: JamiTheme.faddedFontColor

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    text: textMetricsuserUserNameLabel.elidedText
                }

                TextMetrics {
                    id: textMetricsuserUserNameLabel

                    font: userUserNameLabel.font
                    text: userUserNameLabelText
                    elideWidth: userNameOrIdRect.width
                    elide: Qt.ElideMiddle
                }
            }
        }

        Rectangle {
            id: buttonGroup

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: buttonPreferredSize * 3 + 18
            Layout.preferredHeight: buttonPreferredSize
            Layout.rightMargin: 20

            color: "transparent"

            HoverableButton {
                id: startAAudioCallButton

                anchors.right: startAVideoCallButton.left
                anchors.verticalCenter: buttonGroup.verticalCenter

                height: buttonPreferredSize
                width: buttonPreferredSize

                radius: 30
                source: "qrc:/images/icons/ic_phone_24px.svg"
                backgroundColor: "white"
                onExitColor: "white"

                onClicked: {
                    messagingHeaderRect.sendContactRequestButtonClicked()
                    CallAdapter.placeAudioOnlyCall()
                }
            }

            HoverableButton {
                id: startAVideoCallButton

                anchors.right: sendContactRequestButton.left
                anchors.leftMargin: 5
                anchors.verticalCenter: buttonGroup.verticalCenter

                height: buttonPreferredSize
                width: buttonPreferredSize

                radius: 30
                source: "qrc:/images/icons/ic_video_call_24px.svg"
                backgroundColor: "white"
                onExitColor: "white"

                onClicked: {
                    messagingHeaderRect.sendContactRequestButtonClicked()
                    CallAdapter.placeCall()
                }
            }

            HoverableButton {
                id: sendContactRequestButton

                anchors.leftMargin: 5
                anchors.right: buttonGroup.right
                anchors.rightMargin: 8
                anchors.verticalCenter: buttonGroup.verticalCenter

                height: buttonPreferredSize
                width: buttonPreferredSize

                visible: sendContactRequestButtonVisible
                radius: 30
                source: "qrc:/images/icons/ic_person_add_black_24dp_2x.png"
                backgroundColor: "white"
                onExitColor: "white"

                onClicked: {
                    messagingHeaderRect.sendContactRequestButtonClicked()
                    sendContactRequestButtonVisible = false
                }

                onVisibleChanged: {
                    if (sendContactRequestButton.visible) {
                        sendContactRequestButton.width = buttonPreferredSize
                    } else {
                        sendContactRequestButton.width = 0
                    }
                }
            }
        }
    }

    CustomBorder {
        commonBorder: false
        lBorderwidth: 0
        rBorderwidth: 0
        tBorderwidth: 0
        bBorderwidth: 1
        borderColor: JamiTheme.tabbarBorderColor
    }
}
