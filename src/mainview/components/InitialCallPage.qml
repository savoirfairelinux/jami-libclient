/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 *         Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
    id: root

    property bool isIncoming: false
    property bool isAudioOnly: false
    property var accountConvPair: ["", ""]
    property int callStatus: 0
    property string bestName: ""

    signal callCanceled
    signal callAccepted

    color: "black"

    ListModel {
        id: incomingControlsModel
        ListElement { type: "refuse"; image: "qrc:/images/icons/round-close-24px.svg"}
        ListElement { type: "accept"; image: "qrc:/images/icons/check-24px.svg"}
    }
    ListModel {
        id: outgoingControlsModel
        ListElement { type: "cancel"; image: "qrc:/images/icons/round-close-24px.svg"}
    }

    onAccountConvPairChanged: {
        if (accountConvPair[1]) {
            contactImg.updateImage(accountConvPair[1])
            root.bestName = UtilsAdapter.getBestName(accountConvPair[0], accountConvPair[1])
        }
    }

    // Prevent right click propagate to VideoCallPage.
    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        acceptedButtons: Qt.AllButtons
        onDoubleClicked: mouse.accepted = true
    }

    ColumnLayout {
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter

        AvatarImage {
            id: contactImg

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.avatarSizeInCall + spinningAnimationWidth
            Layout.preferredHeight: JamiTheme.avatarSizeInCall + spinningAnimationWidth

            avatarMode: AvatarImage.AvatarMode.FromConvUid
            showPresenceIndicator: false
            spinningAnimationMode: SpinningAnimation.SpinningAnimationMode.NORMAL
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.preferredFieldWidth
            Layout.topMargin: 32

            font.pointSize: JamiTheme.titleFontSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            text: {
                if (root.isIncoming)
                    return root.isAudioOnly ? JamiStrings.incomingAudioCallFrom.replace("{}", root.bestName) : JamiStrings.incomingVideoCallFrom.replace("{}", root.bestName)
                else
                    return root.bestName
            }
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            maximumLineCount: root.isIncoming ? 2 : 1
            color: "white"
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: root.width
            Layout.topMargin: 8

            font.pointSize: JamiTheme.smartlistItemFontSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            text: UtilsAdapter.getCallStatusStr(callStatus) + "…"
            color: JamiTheme.whiteColor
            visible: !root.isIncoming
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 32

            Repeater {
                id: controlButtons
                model: root.isIncoming ? incomingControlsModel : outgoingControlsModel

                delegate: ColumnLayout {
                    PushButton {
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        Layout.alignment: Qt.AlignHCenter
                        implicitWidth: JamiTheme.callButtonPreferredSize
                        implicitHeight: JamiTheme.callButtonPreferredSize

                        pressedColor: {
                            if (type === "accept" )
                                return JamiTheme.acceptGreen
                            return JamiTheme.refuseRed
                        }
                        hoveredColor: {
                            if (type === "accept" )
                                return JamiTheme.acceptGreen
                            return JamiTheme.refuseRed
                        }
                        normalColor: {
                            if (type === "accept" )
                                return JamiTheme.acceptGreenTransparency
                            return JamiTheme.refuseRedTransparent
                        }

                        source: image
                        imageColor: JamiTheme.whiteColor

                        onClicked: {
                            if (type === "accept")
                                callAccepted()
                            else
                                callCanceled()
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.callButtonPreferredSize
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        font.pointSize: JamiTheme.smartlistItemInfoFontSize
                        font.kerning: true
                        color: JamiTheme.whiteColor

                        text: {
                            if (type === "refuse")
                                return JamiStrings.refuse
                            else if (type === "accept")
                                return JamiStrings.accept
                            else if (type === "cancel")
                                return JamiStrings.endCall
                            return ""
                        }

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+Y"
        context: Qt.ApplicationShortcut
        onActivated: {
            CallAdapter.acceptACall(root.accountConvPair[0],
                                    root.accountConvPair[1])
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+D"
        context: Qt.ApplicationShortcut
        onActivated: {
            CallAdapter.hangUpACall(root.accountConvPair[0],
                                    root.accountConvPair[1])
        }
    }
}
