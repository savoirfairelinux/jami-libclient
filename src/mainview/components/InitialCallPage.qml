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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property bool isIncoming: false
    property bool isAudioOnly: false
    property int callStatus: 0

    signal callCanceled
    signal callAccepted

    color: "black"

    ListModel {
        id: incomingControlsModel
        Component.onCompleted: {
            fillIncomingControls()
        }
    }

    Connections {
        target: CurrentAccount

        function onVideoEnabledVideoChanged() {
            fillIncomingControls()
        }
    }

    Connections {
        target: VideoDevices

        function onListSizeChanged() {
            fillIncomingControls()
        }
    }

    function fillIncomingControls() {
            incomingControlsModel.clear()
            incomingControlsModel.append({"type": "refuse", "image": JamiResources.round_close_24dp_svg})
            incomingControlsModel.append({"type": "mic", "image" : JamiResources.place_audiocall_24dp_svg})
            if (CurrentAccount.videoEnabled_Video && VideoDevices.listSize !== 0)
                incomingControlsModel.append({"type": "cam", "image" : JamiResources.videocam_24dp_svg})
    }

    ListModel {
        id: outgoingControlsModel
        Component.onCompleted: {
            append({"type": "cancel", "image": JamiResources.ic_call_end_white_24dp_svg})
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

        ConversationAvatar {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.avatarSizeInCall
            Layout.preferredHeight: JamiTheme.avatarSizeInCall

            showPresenceIndicator: false
            animationMode: SpinningAnimation.Mode.Radial
            imageId: CurrentConversation.id
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: JamiTheme.preferredFieldWidth
            Layout.topMargin: 32

            font.pointSize: JamiTheme.titleFontSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            property string title: CurrentConversation.title

            text: {
                if (root.isIncoming)
                    return root.isAudioOnly ?
                                JamiStrings.incomingAudioCallFrom.replace("{}", title) :
                                JamiStrings.incomingVideoCallFrom.replace("{}", title)
                else
                    return title
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
                    visible: (type === "cam" && root.isAudioOnly) ? false : true;

                    PushButton {
                        id: actionButton
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        Layout.alignment: Qt.AlignHCenter
                        implicitWidth: JamiTheme.callButtonPreferredSize
                        implicitHeight: JamiTheme.callButtonPreferredSize

                        pressedColor: {
                            if ( type === "cam" || type === "mic")
                                return JamiTheme.acceptGreen
                            return JamiTheme.refuseRed
                        }
                        hoveredColor: {
                            if ( type === "cam" || type === "mic")
                                return JamiTheme.acceptGreen
                            return JamiTheme.refuseRed
                        }
                        normalColor: {
                            if ( type === "cam" || type === "mic")
                                return JamiTheme.acceptGreenTransparency
                            return JamiTheme.refuseRedTransparent
                        }

                        source: image
                        imageColor: JamiTheme.whiteColor

                        onClicked: {
                            if ( type === "cam" || type === "mic") {
                                var acceptVideoMedia = true
                                if (type === "cam")
                                    acceptVideoMedia = true
                                else if ( type === "mic" )
                                    acceptVideoMedia = false
                                CallAdapter.setCallMedia(responsibleAccountId, responsibleConvUid, acceptVideoMedia)
                                callAccepted()
                            } else {
                                callCanceled()
                            }
                        }
                    }

                    Label {
                        id: buttonLabel
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.callButtonPreferredSize
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        font.pointSize: JamiTheme.smartlistItemInfoFontSize
                        font.kerning: true
                        color: actionButton.hovered ? JamiTheme.whiteColor : JamiTheme.whiteColorTransparent


                        text: {
                            if (type === "refuse")
                                return JamiStrings.refuse
                            else if (type === "cam")
                                return JamiStrings.acceptVideo
                            else if (type === "mic")
                                return root.isAudioOnly ? JamiStrings.accept : JamiStrings.acceptAudio
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
        onActivated: CallAdapter.acceptACall(CurrentAccount.id,
                                             CurrentConversation.id)
    }

    Shortcut {
        sequence: "Ctrl+Shift+D"
        context: Qt.ApplicationShortcut
        onActivated: {
            CallAdapter.hangUpACall(CurrentAccount.id,
                                    CurrentConversation.id)
        }
    }
}
