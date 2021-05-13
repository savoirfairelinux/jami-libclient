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
    property int photoState: PhotoboothView.PhotoState.Default
    property bool avatarSet: false
    // saveToConfig is to specify whether the image should be saved to account config
    property alias saveToConfig: avatarImg.saveToConfig
    property string fileName: ""

    property int boothWidth: 224

    enum PhotoState {
        Default = 0,
        CameraRendering,
        Taken
    }

    readonly property int size: boothWidth +
                                buttonsRowLayout.height +
                                JamiTheme.preferredMarginSize / 2

    function initUI(useDefaultAvatar = true) {
        photoState = PhotoboothView.PhotoState.Default
        avatarSet = false
        if (useDefaultAvatar)
            setAvatarImage(AvatarImage.Mode.Default, "")
    }

    function startBooth() {
        AccountAdapter.startPreviewing(false)
        photoState = PhotoboothView.PhotoState.CameraRendering
    }

    function stopBooth(){
        try{
            if(!AccountAdapter.hasVideoCall()) {
                AccountAdapter.stopPreviewing()
            }
        } catch(erro){console.log("Exception: " +  erro.message)}
    }

    function setAvatarImage(mode = AvatarImage.Mode.FromAccount,
                            imageId = LRCInstance.currentAccountId){
        if (mode !== AvatarImage.Mode.FromBase64)
            avatarImg.enableAnimation = true
        else
            avatarImg.enableAnimation = false

        avatarImg.mode = mode

        if (mode === AvatarImage.Mode.Default) {
            avatarImg.updateImage(imageId)
            return
        }

        if (imageId)
            avatarImg.updateImage(imageId)
    }

    function manualSaveToConfig() {
        avatarImg.saveAvatarToConfig()
    }

    onVisibleChanged: {
        if(!visible){
            stopBooth()
        }
    }

    spacing: 0

    JamiFileDialog{
        id: importFromFileToAvatar_Dialog

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.chooseAvatarImage
        folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)

        nameFilters: [ qsTr("Image Files") + " (*.png *.jpg *.jpeg)",qsTr(
                "All files") + " (*)"]

        onAccepted: {
            avatarSet = true
            photoState = PhotoboothView.PhotoState.Default

            fileName = file
            if (fileName.length === 0) {
                SettingsAdapter.clearCurrentAvatar()
                setAvatarImage()
                return
            }

            setAvatarImage(AvatarImage.Mode.FromFile,
                           UtilsAdapter.getAbsPath(fileName))
        }
    }

    Label {
        id: avatarLabel

        visible: photoState !== PhotoboothView.PhotoState.CameraRendering

        Layout.fillWidth: true
        Layout.maximumWidth: boothWidth
        Layout.preferredHeight: boothWidth
        Layout.alignment: Qt.AlignHCenter

        background: Rectangle {
            id: avatarLabelBackground

            anchors.fill: parent
            color: "white"
            radius: height / 2

            AvatarImage {
                id: avatarImg

                anchors.fill: parent

                showPresenceIndicator: false

                fillMode: Image.PreserveAspectCrop

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        width: avatarImg.width
                        height: avatarImg.height
                        radius: {
                            var size = ((avatarImg.width <= avatarImg.height) ?
                                            avatarImg.width:avatarImg.height)
                            return size / 2
                        }
                    }
                }

                onImageIsReady: {
                    if (mode === AvatarImage.Mode.FromBase64)
                        photoState = PhotoboothView.PhotoState.Taken

                    if (photoState === PhotoboothView.PhotoState.Taken) {
                        avatarImg.state = ""
                        avatarImg.state = "flashIn"
                    }
                }

                onOpacityChanged: {
                    if (avatarImg.state === "flashIn" && opacity === 0)
                        avatarImg.state = "flashOut"
                }

                states: [
                    State {
                        name: "flashIn"
                        PropertyChanges { target: avatarImg; opacity: 0}
                    }, State {
                        name: "flashOut"
                        PropertyChanges { target: avatarImg; opacity: 1}
                    }]

                transitions: Transition {
                    NumberAnimation {
                        properties: "opacity"
                        easing.type: Easing.Linear
                        duration: 100
                    }
                }
            }
        }
    }

    PhotoboothPreviewRender {
        id:previewWidget

        onHideBooth: stopBooth()

        visible: photoState === PhotoboothView.PhotoState.CameraRendering
        focus: visible

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: boothWidth
        Layout.preferredHeight: boothWidth

        lrcInstance: LRCInstance

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: previewWidget.width
                height: previewWidget.height
                radius: {
                    var size = ((previewWidget.width <= previewWidget.height) ?
                                    previewWidget.width:previewWidget.height)
                    return size / 2
                }
            }
        }
    }

    RowLayout {
        id: buttonsRowLayout

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.topMargin: JamiTheme.preferredMarginSize / 2

        PushButton {
            id: takePhotoButton

            property string cameraAltIconUrl: "qrc:/images/icons/baseline-camera_alt-24px.svg"
            property string addPhotoIconUrl: "qrc:/images/icons/round-add_a_photo-24px.svg"
            property string refreshIconUrl: "qrc:/images/icons/baseline-refresh-24px.svg"

            Layout.alignment: Qt.AlignHCenter

            text: ""
            font.pointSize: 10
            font.kerning: true
            imageColor: JamiTheme.textColor

            toolTipText: JamiStrings.takePhoto

            radius: height / 6
            source: {
                if(photoState === PhotoboothView.PhotoState.Default) {
                    toolTipText = qsTr("Take photo")
                    return cameraAltIconUrl
                }

                if(photoState === PhotoboothView.PhotoState.Taken){
                    toolTipText = qsTr("Retake photo")
                    return refreshIconUrl
                } else {
                    toolTipText = qsTr("Take photo")
                    return addPhotoIconUrl
                }
            }

            onClicked: {
                if(photoState !== PhotoboothView.PhotoState.CameraRendering){
                    startBooth()
                    return
                } else {
                    setAvatarImage(AvatarImage.Mode.FromBase64,
                                   previewWidget.takePhoto(boothWidth))

                    avatarSet = true
                    stopBooth()
                }
            }
        }

        PushButton {
            id: importButton

            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.alignment: Qt.AlignHCenter

            text: ""
            font.pointSize: 10
            font.kerning: true

            radius: height / 6
            source: "qrc:/images/icons/round-folder-24px.svg"

            toolTipText: JamiStrings.importFromFile
            imageColor: JamiTheme.textColor

            onClicked: {
                importFromFileToAvatar_Dialog.open()
            }
        }
    }
}
