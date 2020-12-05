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
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: outgoingCallPageRect

    property int buttonPreferredSize: 50
    property int callStatus: 0
    signal callCancelButtonIsClicked

    function updateUI(accountId, convUid) {
        userInfoCallPage.updateUI(accountId, convUid)
    }

    anchors.fill: parent

    color: "black"

    // Prevent right click propagate to VideoCallPage.
    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        acceptedButtons: Qt.AllButtons
        onDoubleClicked: mouse.accepted = true
    }

    ColumnLayout {
        id: outgoingCallPageRectColumnLayout

        anchors.fill: parent

        UserInfoCallPage {
            id: userInfoCallPage
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        AnimatedImage {
            id: spinnerImage

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 24
            Layout.preferredHeight: 8

            source: "qrc:/images/waiting.gif"
        }

        Text {
            id: callStatusText

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: outgoingCallPageRect.width
            Layout.preferredHeight: 30

            font.pointSize: JamiTheme.textFontSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            text: UtilsAdapter.getCallStatusStr(callStatus) + "…"
            color: Qt.lighter("white", 1.5)
        }

        ColumnLayout {
            id: callCancelButtonColumnLayout

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 48

            PushButton {
                id: callCancelButton

                Layout.alignment: Qt.AlignCenter

                Layout.preferredWidth: buttonPreferredSize
                Layout.preferredHeight: buttonPreferredSize

                pressedColor: JamiTheme.declineButtonPressedRed
                hoveredColor: JamiTheme.declineButtonHoverRed
                normalColor: JamiTheme.declineButtonRed

                source: "qrc:/images/icons/round-close-24px.svg"
                imageColor: JamiTheme.whiteColor

                toolTipText: JamiStrings.hangup

                onClicked: {
                    callCancelButtonIsClicked()
                }
            }
        }
    }
}
