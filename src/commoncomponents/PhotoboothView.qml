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

ColumnLayout {
    id: root

    enum Mode { Static, Previewing }
    property int mode: PhotoboothView.Mode.Static
    property alias imageId: avatar.imageId

    property int size: 224

    signal avatarSet

    function startBooth() {
        AccountAdapter.startPreviewing(false)
        mode = PhotoboothView.Mode.Previewing
    }

    function stopBooth(){
        if (!AccountAdapter.hasVideoCall()) {
            AccountAdapter.stopPreviewing()
        }
        mode = PhotoboothView.Mode.Static
    }

    onVisibleChanged: {
        if (visible) {
            mode = PhotoboothView.Mode.Static
        } else {
            stopBooth()
        }
    }

    spacing: 0

    JamiFileDialog {
        id: importFromFileDialog

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.chooseAvatarImage
        folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)

        nameFilters: [
            qsTr("Image Files") + " (*.png *.jpg *.jpeg)",
            qsTr("All files") + " (*)"
        ]

        onAccepted: {
            var filePath = UtilsAdapter.getAbsPath(file)
            AccountAdapter.setCurrentAccountAvatarFile(filePath)
            avatarSet()
        }
    }

    Item {
        id: imageLayer

        Layout.preferredWidth: size
        Layout.preferredHeight: size
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

            visible: mode === PhotoboothView.Mode.Previewing

            onRenderingStopped: stopBooth()
            lrcInstance: LRCInstance

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: size
                    height: size
                    radius: size / 2
                }
            }
        }

        Rectangle {
            id: flashRect

            anchors.fill: parent
            anchors.margins: 0
            radius: size / 2
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
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.topMargin: JamiTheme.preferredMarginSize / 2
        Layout.alignment: Qt.AlignHCenter

        PushButton {
            id: takePhotoButton

            Layout.alignment: Qt.AlignHCenter
            radius: JamiTheme.primaryRadius

            imageColor: JamiTheme.textColor
            toolTipText: JamiStrings.takePhoto

            source: mode === PhotoboothView.Mode.Static ?
                        "qrc:/images/icons/baseline-camera_alt-24px.svg" :
                        "qrc:/images/icons/round-add_a_photo-24px.svg"

            onClicked: {
                if (mode === PhotoboothView.Mode.Previewing) {
                    flashAnimation.start()
                    AccountAdapter.setCurrentAccountAvatarBase64(
                                preview.takePhoto(size))
                    avatarSet()
                    stopBooth()
                    return
                }

                startBooth()
            }
        }

        PushButton {
            id: importButton

            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.alignment: Qt.AlignHCenter

            radius: JamiTheme.primaryRadius
            source: "qrc:/images/icons/round-folder-24px.svg"

            toolTipText: JamiStrings.importFromFile
            imageColor: JamiTheme.textColor

            onClicked: {
                stopBooth()
                importFromFileDialog.open()
            }
        }
    }
}
