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
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

Item {
    id: root

    property bool isPreviewing: false
    property alias imageId: avatar.imageId
    required property real avatarSize

    signal focusOnPreviousItem
    signal focusOnNextItem

    width: avatarSize
    height: boothLayout.height

    function startBooth() {
        AccountAdapter.startPreviewing(false)
        isPreviewing = true
    }

    function stopBooth(){
        if (!AccountAdapter.hasVideoCall()) {
            AccountAdapter.stopPreviewing()
        }
        isPreviewing = false
    }

    function focusOnNextPhotoBoothItem () {
        takePhotoButton.forceActiveFocus()
    }

    function focusOnPreviousPhotoBoothItem () {
        if (isPreviewing)
            clearButton.forceActiveFocus()
        else
            importButton.forceActiveFocus()
    }

    onVisibleChanged: {
        if (!visible) {
            stopBooth()
        }
    }

    JamiFileDialog {
        id: importFromFileDialog

        objectName: "photoboothImportFromFileDialog"

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.chooseAvatarImage
        folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)

        nameFilters: [
            qsTr("Image Files") + " (*.png *.jpg *.jpeg)",
            qsTr("All files") + " (*)"
        ]

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

        onAccepted: {
            if (importButton.focusAfterFileDialogClosed) {
                importButton.focusAfterFileDialogClosed = false
                importButton.forceActiveFocus()
            }

            var filePath = UtilsAdapter.getAbsPath(file)
            AccountAdapter.setCurrentAccountAvatarFile(filePath)
        }

        onRejected: {
            if (importButton.focusAfterFileDialogClosed) {
                importButton.focusAfterFileDialogClosed = false
                importButton.forceActiveFocus()
            }
        }
    }

    ColumnLayout {
        id: boothLayout

        spacing: JamiTheme.preferredMarginSize / 2

        Item {
            id: imageLayer

            Layout.preferredWidth: avatarSize
            Layout.preferredHeight: avatarSize
            Layout.alignment: Qt.AlignHCenter

            Avatar {
                id: avatar

                anchors.fill: parent
                anchors.margins: 1

                visible: !preview.visible

                fillMode: Image.PreserveAspectCrop
                showPresenceIndicator: false
            }

            PhotoboothPreviewRender {
                id: preview

                anchors.fill: parent
                anchors.margins: 1

                visible: isPreviewing

                onRenderingStopped: stopBooth()
                lrcInstance: LRCInstance

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        width: avatarSize
                        height: avatarSize
                        radius: avatarSize / 2
                    }
                }
            }

            Rectangle {
                id: flashRect

                anchors.fill: parent
                anchors.margins: 0
                radius: avatarSize / 2
                color: "white"
                opacity: 0

                SequentialAnimation {
                    id: flashAnimation

                    NumberAnimation {
                        target: flashRect; property: "opacity"
                        to: 1; duration: 0
                    }
                    NumberAnimation {
                        target: flashRect; property: "opacity"
                        to: 0; duration: 500
                    }
                }
            }
        }

        RowLayout {
            id: buttonsRowLayout

            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height
            Layout.bottomMargin: parent.spacing
            Layout.alignment: Qt.AlignHCenter

            PushButton {
                id: takePhotoButton

                objectName: "takePhotoButton"

                Layout.alignment: Qt.AlignHCenter

                radius: JamiTheme.primaryRadius
                imageColor: JamiTheme.textColor
                toolTipText: JamiStrings.takePhoto
                source: isPreviewing ?
                            JamiResources.round_add_a_photo_24dp_svg :
                            JamiResources.baseline_camera_alt_24dp_svg

                Keys.onPressed: function (keyEvent) {
                    if (keyEvent.key === Qt.Key_Enter ||
                            keyEvent.key === Qt.Key_Return) {
                        clicked()
                        keyEvent.accepted = true
                    } else if (keyEvent.key === Qt.Key_Up) {
                        root.focusOnPreviousItem()
                        keyEvent.accepted = true
                    }
                }

                KeyNavigation.tab: {
                    if (clearButton.visible)
                        return clearButton
                    return importButton
                }
                KeyNavigation.down: KeyNavigation.tab

                onClicked: {
                    if (isPreviewing) {
                        flashAnimation.start()
                        AccountAdapter.setCurrentAccountAvatarBase64(
                                    preview.takePhoto(avatarSize))
                        stopBooth()
                        return
                    }

                    startBooth()
                }
            }

            PushButton {
                id: clearButton

                objectName: "photoboothViewClearButton"

                Layout.alignment: Qt.AlignHCenter

                visible: isPreviewing || LRCInstance.currentAccountAvatarSet

                radius: JamiTheme.primaryRadius
                source: JamiResources.round_close_24dp_svg
                toolTipText: isPreviewing ? JamiStrings.stopTakingPhoto :
                                            JamiStrings.clearAvatar
                imageColor: JamiTheme.textColor

                KeyNavigation.up: takePhotoButton

                Keys.onPressed: function (keyEvent) {
                    if (keyEvent.key === Qt.Key_Enter ||
                            keyEvent.key === Qt.Key_Return) {
                        clicked()
                        takePhotoButton.forceActiveFocus()
                        keyEvent.accepted = true
                    } else if (keyEvent.key === Qt.Key_Down ||
                               keyEvent.key === Qt.Key_Tab) {
                        if (isPreviewing) {
                            root.focusOnNextItem()
                        } else
                            importButton.forceActiveFocus()
                        keyEvent.accepted = true
                    }
                }

                onClicked: {
                    stopBooth()
                    if (!isPreviewing)
                        AccountAdapter.setCurrentAccountAvatarBase64()
                }
            }

            PushButton {
                id: importButton

                objectName: "photoboothViewImportButton"

                property bool focusAfterFileDialogClosed: false

                Layout.alignment: Qt.AlignHCenter

                visible: !isPreviewing

                radius: JamiTheme.primaryRadius
                source: JamiResources.round_folder_24dp_svg
                toolTipText: JamiStrings.importFromFile
                imageColor: JamiTheme.textColor

                Keys.onPressed: function (keyEvent) {
                    if (keyEvent.key === Qt.Key_Enter ||
                            keyEvent.key === Qt.Key_Return) {
                        focusAfterFileDialogClosed = true
                        clicked()
                        keyEvent.accepted = true
                    } else if (keyEvent.key === Qt.Key_Down ||
                               keyEvent.key === Qt.Key_Tab) {
                        root.focusOnNextItem()
                        keyEvent.accepted = true
                    }
                }

                KeyNavigation.up: {
                    if (clearButton.visible)
                        return clearButton
                    return takePhotoButton
                }

                onClicked: {
                    stopBooth()
                    importFromFileDialog.open()
                }
            }
        }
    }
}
