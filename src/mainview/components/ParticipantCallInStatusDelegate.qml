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

import QtQuick
import QtQuick.Layouts

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

SpinningAnimation {
    id: root

    width: contentRect.width + spinningAnimationWidth
    height: JamiTheme.participantCallInStatusDelegateHeight

    mode: SpinningAnimation.Mode.BiRadial
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

        Avatar {
            id: avatar

            anchors.left: contentRect.left
            anchors.leftMargin: 10
            anchors.verticalCenter: contentRect.verticalCenter

            width: JamiTheme.participantCallInAvatarSize
            height: JamiTheme.participantCallInAvatarSize

            showPresenceIndicator: false
            mode: Avatar.Mode.Contact
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
                Layout.preferredWidth: callStatus.Layout.preferredWidth

                font.weight: Font.Bold
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

            source: JamiResources.cross_black_24dp_svg
            imageColor: JamiTheme.whiteColor

            toolTipText: JamiStrings.optionCancel

            onClicked: CallAdapter.hangUpCall(PendingConferenceeCallId)
        }
    }
}
