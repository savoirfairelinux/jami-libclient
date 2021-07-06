/*
 * Copyright (C) 2021 by Savoir-faire Linux
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

import QtTest 1.2

import net.jami.Models 1.1
import net.jami.Constants 1.1

import "qrc:/src/mainview/components"

ColumnLayout {
    id: root

    spacing: 0

    width: 300
    height: uut.implicitHeight

    ChatViewFooter {
        id: uut

        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.preferredHeight: implicitHeight
        Layout.maximumHeight: JamiTheme.chatViewMaximumWidth

        TestCase {
            name: "MessageWebViewFooter Send Message Button Visibility Test"
            when: windowShown

            function test_send_message_button_visibility() {
                var filesToSendContainer = findChild(uut, "dataTransferSendContainer")
                var sendMessageButton = findChild(uut, "sendMessageButton")
                var messageBarTextArea = findChild(uut, "messageBarTextArea")

                compare(sendMessageButton.visible, false)

                // Text in messageBarTextArea will cause sendMessageButton to show
                messageBarTextArea.insertText("test")
                compare(sendMessageButton.visible, true)

                // Text cleared in messageBarTextArea will cause sendMessageButton to hide
                messageBarTextArea.clearText()
                compare(sendMessageButton.visible, false)

                // File added into filesToSendContainer will cause sendMessageButton to show
                filesToSendContainer.filesToSendListModel.addToPending(
                            ":/src/resources/png_test.png")
                compare(filesToSendContainer.filesToSendCount, 1)
                compare(sendMessageButton.visible, true)

                // Files cleared from filesToSendContainer will cause sendMessageButton to hide
                filesToSendContainer.filesToSendListModel.flush()
                compare(filesToSendContainer.filesToSendCount, 0)
                compare(sendMessageButton.visible, false)

                // When the text and files both exist,
                // clear one of them will still make sendMessageButton to show
                messageBarTextArea.insertText("test")
                filesToSendContainer.filesToSendListModel.addToPending(
                            ":/src/resources/png_test.png")
                messageBarTextArea.clearText()
                compare(sendMessageButton.visible, true)

                messageBarTextArea.insertText("test")
                filesToSendContainer.filesToSendListModel.flush()
                compare(sendMessageButton.visible, true)

                // Both are cleared
                messageBarTextArea.clearText()
                compare(sendMessageButton.visible, false)
            }
        }
    }
}
