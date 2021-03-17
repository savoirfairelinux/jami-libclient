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
import QtQuick.Controls.Universal 2.14
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: incomingCallPage

    property int buttonPreferredSize: 48

    signal callCancelButtonIsClicked
    signal callAcceptButtonIsClicked

    color: "black"

    function updateUI(accountId, convUid) {
        userInfoIncomingCallPage.updateUI(accountId, convUid)
    }

    // Prevent right click propagate to VideoCallPage.
    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        acceptedButtons: Qt.AllButtons
        onDoubleClicked: mouse.accepted = true
    }

    ColumnLayout {
        id: incomingCallPageColumnLayout

        anchors.fill: parent

        // Common elements with OutgoingCallPage
        UserInfoCallPage {
            id: userInfoIncomingCallPage
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Text {
            id: talkToYouText

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: incomingCallPage.width
            Layout.preferredHeight: 32

            font.pointSize: JamiTheme.textFontSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "white"

            text: "is calling you"
        }

        RowLayout {
            id: incomingCallPageRowLayout

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.bottomMargin: 48
            Layout.topMargin: 48

            Layout.preferredWidth: incomingCallPage.width - 200
            Layout.maximumWidth: 200
            Layout.preferredHeight: buttonPreferredSize

            ColumnLayout {
                id: callAnswerButtonColumnLayout

                Layout.alignment: Qt.AlignLeft

                PushButton {
                    id: callAnswerButton

                    Layout.alignment: Qt.AlignCenter

                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize

                    pressedColor: JamiTheme.acceptButtonPressedGreen
                    hoveredColor: JamiTheme.acceptButtonHoverGreen
                    normalColor: JamiTheme.acceptButtonGreen

                    source: "qrc:/images/icons/check-24px.svg"
                    imageColor: JamiTheme.whiteColor

                    onClicked: callAcceptButtonIsClicked()
                }
            }

            ColumnLayout {
                id: callDeclineButtonColumnLayout

                Layout.alignment: Qt.AlignRight

                PushButton {
                    id: callDeclineButton

                    Layout.alignment: Qt.AlignCenter

                    Layout.preferredWidth: buttonPreferredSize
                    Layout.preferredHeight: buttonPreferredSize

                    pressedColor: JamiTheme.declineButtonPressedRed
                    hoveredColor: JamiTheme.declineButtonHoverRed
                    normalColor: JamiTheme.declineButtonRed

                    source: "qrc:/images/icons/round-close-24px.svg"
                    imageColor: JamiTheme.whiteColor

                    toolTipText: JamiStrings.hangup

                    onClicked: callCancelButtonIsClicked()
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+Y"
        context: Qt.ApplicationShortcut
        onActivated: {
            incomingCallPage.close()
            CallAdapter.acceptACall(responsibleAccountId,
                                    responsibleConvUid)
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
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
