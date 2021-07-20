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

ColumnLayout {
    id: root

    property alias text: textArea.text
    property var textAreaObj: textArea
    property real marginSize: 10
    property bool sendButtonVisibility: false
    property bool animate: false

    signal sendMessageButtonClicked
    signal sendFileButtonClicked
    signal audioRecordMessageButtonClicked
    signal videoRecordMessageButtonClicked
    signal emojiButtonClicked

    implicitHeight: messageBarRowLayout.height

    spacing: 0

    Rectangle {
        id: messageBarHairLine

        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
        Layout.preferredHeight: JamiTheme.messageWebViewHairLineSize
        Layout.fillWidth: true
        Layout.maximumWidth: JamiTheme.messageWebViewFooterContentMaximumWidth

        color: JamiTheme.tabbarBorderColor
    }

    RowLayout {
        id: messageBarRowLayout

        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        Layout.maximumWidth: JamiTheme.messageWebViewFooterContentMaximumWidth

        spacing: JamiTheme.messageWebViewFooterRowSpacing

        PushButton {
            id: sendFileButton

            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: marginSize
            Layout.preferredWidth: JamiTheme.messageWebViewFooterButtonSize
            Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

            radius: JamiTheme.messageWebViewFooterButtonRadius
            preferredSize: JamiTheme.messageWebViewFooterButtonIconSize - 6

            toolTipText: JamiStrings.sendFile

            source: JamiResources.link_black_24dp_svg

            normalColor: JamiTheme.primaryBackgroundColor
            imageColor: JamiTheme.messageWebViewFooterButtonImageColor

            onClicked: root.sendFileButtonClicked()
        }

        PushButton {
            id: audioRecordMessageButton

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: JamiTheme.messageWebViewFooterButtonSize
            Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

            radius: JamiTheme.messageWebViewFooterButtonRadius
            preferredSize: JamiTheme.messageWebViewFooterButtonIconSize

            toolTipText: JamiStrings.leaveAudioMessage

            source: JamiResources.message_audio_black_24dp_svg

            normalColor: JamiTheme.primaryBackgroundColor
            imageColor: JamiTheme.messageWebViewFooterButtonImageColor

            onClicked: root.audioRecordMessageButtonClicked()

            Component.onCompleted: JamiQmlUtils.audioRecordMessageButtonObj = audioRecordMessageButton
        }

        PushButton {
            id: videoRecordMessageButton

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: JamiTheme.messageWebViewFooterButtonSize
            Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

            radius: JamiTheme.messageWebViewFooterButtonRadius
            preferredSize: JamiTheme.messageWebViewFooterButtonIconSize

            toolTipText: JamiStrings.leaveVideoMessage

            source: JamiResources.message_video_black_24dp_svg

            normalColor: JamiTheme.primaryBackgroundColor
            imageColor: JamiTheme.messageWebViewFooterButtonImageColor

            onClicked: root.videoRecordMessageButtonClicked()

            Component.onCompleted: JamiQmlUtils.videoRecordMessageButtonObj = videoRecordMessageButton
        }

        MessageBarTextArea {
            id: textArea

            objectName: "messageBarTextArea"

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.margins: marginSize / 2
            Layout.preferredHeight: {
                return JamiTheme.messageWebViewFooterPreferredHeight
                        > contentHeight ? JamiTheme.messageWebViewFooterPreferredHeight : contentHeight
            }
            Layout.maximumHeight: JamiTheme.messageWebViewFooterTextAreaMaximumHeight
                                  - marginSize / 2

            onSendMessagesRequired: root.sendMessageButtonClicked()
        }

        PushButton {
            id: emojiButton

            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: sendMessageButton.visible ? 0 : marginSize
            Layout.preferredWidth: JamiTheme.messageWebViewFooterButtonSize
            Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

            radius: JamiTheme.messageWebViewFooterButtonRadius
            preferredSize: JamiTheme.messageWebViewFooterButtonIconSize

            toolTipText: JamiStrings.addEmoji

            source: JamiResources.emoji_black_24dp_svg

            normalColor: JamiTheme.primaryBackgroundColor
            imageColor: JamiTheme.messageWebViewFooterButtonImageColor

            onClicked: root.emojiButtonClicked()

            Component.onCompleted: JamiQmlUtils.emojiPickerButtonObj = emojiButton
        }

        PushButton {
            id: sendMessageButton

            objectName: "sendMessageButton"

            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: visible ? marginSize : 0
            Layout.preferredWidth: scale * JamiTheme.messageWebViewFooterButtonSize
            Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

            radius: JamiTheme.messageWebViewFooterButtonRadius
            preferredSize: JamiTheme.messageWebViewFooterButtonIconSize - 6

            toolTipText: JamiStrings.send

            source: JamiResources.send_black_24dp_svg

            normalColor: JamiTheme.primaryBackgroundColor
            imageColor: JamiTheme.messageWebViewFooterButtonImageColor

            opacity: sendButtonVisibility ? 1 : 0
            visible: opacity
            scale: opacity

            Behavior on opacity {
                enabled: animate
                NumberAnimation {
                    duration: JamiTheme.shortFadeDuration
                    easing.type: Easing.InOutQuad
                }
            }

            onClicked: root.sendMessageButtonClicked()
        }

        Component.onCompleted: JamiQmlUtils.messageBarButtonsRowObj = messageBarRowLayout
    }
}
