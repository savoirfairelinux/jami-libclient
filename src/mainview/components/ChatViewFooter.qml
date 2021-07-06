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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Constants 1.1
import net.jami.Adapters 1.1

import "../../commoncomponents"
import "../../commoncomponents/emojipicker"

Rectangle {
    id: root

    property alias textInput: messageBar.textAreaObj
    property string previousConvId: ""

    function setFilePathsToSend(filePaths) {
        for (var index = 0; index < filePaths.length; ++index) {
            var path = UtilsAdapter.getAbsPath(filePaths[index])
            dataTransferSendContainer.filesToSendListModel.addToPending(path)
        }
    }

    implicitHeight: footerColumnLayout.implicitHeight

    color: JamiTheme.primaryBackgroundColor

    Connections {
        target: LRCInstance

        function onSelectedConvUidChanged() {
            // Handle Draft
            if (previousConvId !== "") {
                LRCInstance.setContentDraft(previousConvId, LRCInstance.currentAccountId,
                                            messageBar.text);
            }

            // turn off the button animations when switching convs
            messageBar.animate = false

            messageBar.textAreaObj.clearText()
            previousConvId = LRCInstance.selectedConvUid

            var restoredContent = LRCInstance.getContentDraft(LRCInstance.selectedConvUid,
                                                              LRCInstance.currentAccountId);
            if (restoredContent)
                messageBar.textAreaObj.insertText(restoredContent)

            messageBar.animate = true
        }
    }

    Connections {
        target: MessagesAdapter

        function onNewMessageBarPlaceholderText(placeholderText) {
            messageBar.textAreaObj.placeholderText = JamiStrings.writeTo.arg(placeholderText)
        }

        function onNewFilePasted(filePath) {
            dataTransferSendContainer.filesToSendListModel.addToPending(filePath)
        }

        function onNewTextPasted() {
            messageBar.textAreaObj.pasteText()
        }
    }

    RecordBox{
        id: recordBox

        visible: false
    }

    EmojiPicker {
        id: emojiPicker

        onEmojiIsPicked: messageBar.textAreaObj.insertText(content)
    }

    JamiFileDialog {
        id: jamiFileDialog

        mode: JamiFileDialog.Mode.OpenFiles

        onAccepted: setFilePathsToSend(jamiFileDialog.files)
    }

    ColumnLayout {
        id: footerColumnLayout

        anchors.centerIn: root

        width: root.width

        spacing: 0

        MessageBar {
            id: messageBar

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: footerColumnLayout.width
            Layout.preferredHeight: implicitHeight

            sendButtonVisibility: text ||
                                  dataTransferSendContainer.filesToSendCount

            onEmojiButtonClicked: {
                JamiQmlUtils.updateMessageBarButtonsPoints()

                emojiPicker.parent = JamiQmlUtils.mainViewRectObj

                emojiPicker.x = Qt.binding(function() {
                    var buttonX = JamiQmlUtils.emojiPickerButtonInMainViewPoint.x +
                            JamiQmlUtils.emojiPickerButtonObj.width
                    return buttonX - emojiPicker.width
                })
                emojiPicker.y = Qt.binding(function() {
                    var buttonY = JamiQmlUtils.audioRecordMessageButtonInMainViewPoint.y
                    return buttonY - emojiPicker.height - messageBar.marginSize
                            - JamiTheme.chatViewHairLineSize
                })

                emojiPicker.openEmojiPicker()
            }
            onSendFileButtonClicked: jamiFileDialog.open()
            onSendMessageButtonClicked: {
                // Send text message
                if (messageBar.text)
                    MessagesAdapter.sendMessage(messageBar.text)
                messageBar.textAreaObj.clearText()

                // Send file messages
                var fileCounts = dataTransferSendContainer.filesToSendListModel.rowCount()
                for (var i = 0; i < fileCounts; i++) {
                    var currentIndex = dataTransferSendContainer.filesToSendListModel.index(i, 0)
                    var filePath = dataTransferSendContainer.filesToSendListModel.data(
                                currentIndex, FilesToSend.FilePath)
                    MessagesAdapter.sendFile(filePath)
                }
                dataTransferSendContainer.filesToSendListModel.flush()
            }
            onVideoRecordMessageButtonClicked: {
                JamiQmlUtils.updateMessageBarButtonsPoints()

                recordBox.parent = JamiQmlUtils.mainViewRectObj

                recordBox.x = Qt.binding(function() {
                    var buttonCenterX = JamiQmlUtils.videoRecordMessageButtonInMainViewPoint.x +
                            JamiQmlUtils.videoRecordMessageButtonObj.width / 2
                    return buttonCenterX - recordBox.width / 2
                })
                recordBox.y = Qt.binding(function() {
                    var buttonY = JamiQmlUtils.videoRecordMessageButtonInMainViewPoint.y
                    return buttonY - recordBox.height - recordBox.spikeHeight
                })

                recordBox.openRecorder(true)
            }
            onAudioRecordMessageButtonClicked: {
                JamiQmlUtils.updateMessageBarButtonsPoints()

                recordBox.parent = JamiQmlUtils.mainViewRectObj

                recordBox.x = Qt.binding(function() {
                    var buttonCenterX = JamiQmlUtils.audioRecordMessageButtonInMainViewPoint.x +
                            JamiQmlUtils.audioRecordMessageButtonObj.width / 2
                    return buttonCenterX - recordBox.width / 2
                })
                recordBox.y = Qt.binding(function() {
                    var buttonY = JamiQmlUtils.audioRecordMessageButtonInMainViewPoint.y
                    return buttonY - recordBox.height - recordBox.spikeHeight
                })

                recordBox.openRecorder(false)
            }
        }

        FilesToSendContainer {
            id: dataTransferSendContainer

            objectName: "dataTransferSendContainer"

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: footerColumnLayout.width
            Layout.maximumWidth: JamiTheme.chatViewMaximumWidth
            Layout.preferredHeight: filesToSendCount ?
                                        JamiTheme.chatViewFooterFileContainerPreferredHeight : 0
        }
    }
}
