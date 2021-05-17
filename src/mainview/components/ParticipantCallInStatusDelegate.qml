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

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

SpinningAnimation {
    id: root

    width: contentRect.width + spinningAnimationWidth
    height: JamiTheme.participantCallInStatusDelegateHeight

    spinningAnimationMode: SpinningAnimation.SpinningAnimationMode.SYMMETRY
    outerCutRadius: JamiTheme.participantCallInStatusDelegateRadius
    spinningAnimationDuration: 5000

    Rectangle {
        id: contentRect

        anchors.centerIn: root

        width: JamiTheme.participantCallInStatusViewWidth + callStatus.Layout.preferredWidth
               - JamiTheme.participantCallInStatusTextWidth - spinningAnimationWidth
        height: JamiTheme.participantCallInStatusDelegateHeight - spinningAnimationWidth

        color: JamiTheme.darkGreyColor
        opacity: JamiTheme.participantCallInStatusOpacity
        radius: JamiTheme.participantCallInStatusDelegateRadius

        AvatarImage {
            id: avatar

            anchors.left: contentRect.left
            anchors.leftMargin: 10
            anchors.verticalCenter: contentRect.verticalCenter

            width: JamiTheme.participantCallInAvatarSize
            height: JamiTheme.participantCallInAvatarSize

            showPresenceIndicator: false
            avatarMode: AvatarImage.AvatarMode.FromContactUri
            imageId: ContactUri
        }

        ColumnLayout {
            id: infoColumnLayout

            anchors.left: avatar.right
            anchors.leftMargin: 5
            anchors.verticalCenter: contentRect.verticalCenter

            implicitHeight: 50
            implicitWidth: JamiTheme.participantCallInStatusTextWidth

            spacing: 5

            Text {
                id: name

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.preferredWidth: JamiTheme.participantCallInStatusTextWidth

                font.weight: Font.ExtraBold
                font.pointSize: JamiTheme.participantCallInNameFontSize
                color: JamiTheme.participantCallInStatusTextColor
                text: PrimaryName
                elide: Text.ElideRight
            }

            Text {
                id: callStatus

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                font.weight: Font.Normal
                font.pointSize: JamiTheme.participantCallInStatusFontSize
                color: JamiTheme.participantCallInStatusTextColor
                text: CallStatus + "…"
                elide: Text.ElideRight

                onWidthChanged: {
                    if (width > JamiTheme.participantCallInStatusTextWidth
                            && width < JamiTheme.participantCallInStatusTextWidthLimit)
                        callStatus.Layout.preferredWidth = width
                    else if (width >= JamiTheme.participantCallInStatusTextWidthLimit)
                        callStatus.Layout.preferredWidth
                                = JamiTheme.participantCallInStatusTextWidthLimit
                    else
                        callStatus.Layout.preferredWidth
                                = JamiTheme.participantCallInStatusTextWidth
                }
            }
        }

        PushButton {
            id: callCancelButton

            anchors.right: contentRect.right
            anchors.rightMargin: 10
            anchors.verticalCenter: contentRect.verticalCenter

            width: 40
            height: 40
            // To control the size of the svg
            preferredSize: 50

            pressedColor: JamiTheme.refuseRed
            hoveredColor: JamiTheme.refuseRed
            normalColor: JamiTheme.refuseRedTransparent

            source: "qrc:/images/icons/cross_black_24dp.svg"
            imageColor: JamiTheme.whiteColor

            toolTipText: JamiStrings.optionCancel

            onClicked: CallAdapter.hangUpCall(PendingConferenceeCallId)
        }
    }
}
