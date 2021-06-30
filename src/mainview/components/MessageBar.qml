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
import net.jami.Constants 1.0

import "../../commoncomponents"

RowLayout {
    id: root

    property var textAreaObj: textArea
    property real marginSize: 10

    signal sendFileButtonClicked
    signal audioRecordMessageButtonClicked
    signal videoRecordMessageButtonClicked
    signal emojiButtonClicked

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

        source: "qrc:/images/icons/link_black-24dp.svg"

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

        source: "qrc:/images/icons/message_audio_black-24dp.svg"

        normalColor: JamiTheme.primaryBackgroundColor
        imageColor: JamiTheme.messageWebViewFooterButtonImageColor

        onClicked: root.audioRecordMessageButtonClicked()
    }

    PushButton {
        id: videoRecordMessageButton

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: JamiTheme.messageWebViewFooterButtonSize
        Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

        radius: JamiTheme.messageWebViewFooterButtonRadius
        preferredSize: JamiTheme.messageWebViewFooterButtonIconSize

        toolTipText: JamiStrings.leaveVideoMessage

        source: "qrc:/images/icons/message_video_black-24dp.svg"

        normalColor: JamiTheme.primaryBackgroundColor
        imageColor: JamiTheme.messageWebViewFooterButtonImageColor

        onClicked: root.videoRecordMessageButtonClicked()
    }

    JamiTextArea {
        id: textArea

        Layout.alignment: Qt.AlignVCenter
        Layout.fillWidth: true
        Layout.margins: marginSize / 2
        Layout.preferredHeight: {
            return JamiTheme.messageWebViewFooterPreferredHeight
                    > contentHeight ? JamiTheme.messageWebViewFooterPreferredHeight : contentHeight
        }
        Layout.maximumHeight: JamiTheme.messageWebViewFooterTextAreaMaximumHeight
                              - marginSize / 2

        onTextChanged: {
            if (text) {
                sendMessageButton.visible = true
                sendMessageButton.state = "buttonFadeOut"
            } else
                sendMessageButton.state = "buttonFadeIn"
        }
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

        source: "qrc:/images/icons/emoji_black-24dp.svg"

        normalColor: JamiTheme.primaryBackgroundColor
        imageColor: JamiTheme.messageWebViewFooterButtonImageColor

        onClicked: root.emojiButtonClicked()
    }

    PushButton {
        id: sendMessageButton

        Layout.alignment: Qt.AlignVCenter
        Layout.rightMargin: visible ? marginSize : 0
        Layout.preferredWidth: JamiTheme.messageWebViewFooterButtonSize
        Layout.preferredHeight: JamiTheme.messageWebViewFooterButtonSize

        radius: JamiTheme.messageWebViewFooterButtonRadius
        preferredSize: JamiTheme.messageWebViewFooterButtonIconSize - 6

        toolTipText: JamiStrings.send

        source: "qrc:/images/icons/send_black-24dp.svg"

        normalColor: JamiTheme.primaryBackgroundColor
        imageColor: JamiTheme.messageWebViewFooterButtonImageColor

        opacity: 0
        visible: false
        states: [
            State {
                name: "buttonFadeIn"
                PropertyChanges {
                    target: sendMessageButton
                    opacity: 0
                }
            },
            State {
                name: "buttonFadeOut"
                PropertyChanges {
                    target: sendMessageButton
                    opacity: 1
                }
            }
        ]
        transitions: Transition {
            NumberAnimation {
                properties: "opacity"
                easing.type: Easing.InOutQuad
                duration: 300
            }
        }

        onOpacityChanged: {
            if (opacity === 0)
                visible = false
        }

        onClicked: {

        }
    }
}
