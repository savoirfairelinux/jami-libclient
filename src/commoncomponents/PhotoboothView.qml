import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.0
import net.jami.Models 1.0

ColumnLayout{
    property bool takePhotoState: false
    property bool hasAvatar: false
    property bool isDefaultIcon: false
    property string imgBase64: ""
    property string fileName: ""

    readonly property int preferredWidth: boothWidth + buttonsRowLayout.height
    property int boothWidth: 224

    signal imageAcquired
    signal imageCleared

    function startBooth(force = false){
        hasAvatar = false
        ClientWrapper.accountAdaptor.startPreviewing(force)
        takePhotoState = true
    }

    function stopBooth(){
        try{
            if(!ClientWrapper.accountAdaptor.hasVideoCall()) {
                ClientWrapper.accountAdaptor.stopPreviewing()
            }
        } catch(erro){console.log("Exception: " +  erro.message)}

        takePhotoState = false
    }

    function setAvatarPixmap(avatarPixmapBase64, defaultValue = false){
        imgBase64 = avatarPixmapBase64
        stopBooth()
        if(defaultValue){
            isDefaultIcon = defaultValue
        }
    }

    onVisibleChanged: {
        if(!visible){
            stopBooth()
        }
    }

    JamiFileDialog{
        id: importFromFileToAvatar_Dialog

        mode: JamiFileDialog.OpenFile
        title: qsTr("Choose an image to be the avatar")
        folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)

        nameFilters: [ qsTr("Image Files") + " (*.png *.jpg *.jpeg)",qsTr(
                "All files") + " (*)"]

        onAccepted: {
            fileName = file
            if(fileName.length === 0) {
                imageCleared()
                return
            }
            imgBase64 = ClientWrapper.utilsAdaptor.getCroppedImageBase64FromFile(
                        ClientWrapper.utilsAdaptor.getAbsPath(fileName),boothWidth)
            imageAcquired()
            stopBooth()
        }
    }

    spacing: 0

    Label{
        id: avatarLabel

        visible: !takePhotoState

        Layout.preferredWidth: boothWidth
        Layout.preferredHeight: boothWidth
        Layout.alignment: Qt.AlignHCenter

        background: Rectangle {
            id: avatarLabelBackground

            anchors.fill: parent
            color: "grey"
            radius: height / 2

            Image{
                id: avatarImg

                anchors.fill: parent
                source:  {
                    if(imgBase64.length === 0){
                        return "qrc:/images/default_avatar_overlay.svg"
                    } else {
                        return "data:image/png;base64," + imgBase64
                    }
                }
                fillMode: Image.PreserveAspectCrop
                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle{
                        width: avatarImg.width
                        height: avatarImg.height
                        radius: {
                            var size = ((avatarImg.width <= avatarImg.height)? avatarImg.width:avatarImg.height)
                            return size /2
                        }
                    }
                }
            }
        }
    }

    PhotoboothPreviewRender{
        id:previewWidget

        onHideBooth:{
            stopBooth()
        }
        visible: takePhotoState
        focus: visible

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: boothWidth
        Layout.preferredHeight: boothWidth

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle{
                width: previewWidget.width
                height: previewWidget.height
                radius: {
                    var size = ((previewWidget.width <= previewWidget.height)? previewWidget.width:previewWidget.height)
                    return size /2
                }
            }
        }

        Label{
            id: flashOverlay

            anchors.fill: previewWidget
            visible: false
            color: "#fff"

            OpacityAnimator on opacity{
                id: flashAnimation

                from: 1
                to: 0
                duration: 600
            }
        }
    }

    RowLayout{
        id: buttonsRowLayout

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 30
        Layout.topMargin: 5

        spacing: 15

        HoverableButton {
            id: takePhotoButton

            property string cameraAltIconUrl: "qrc:/images/icons/baseline-camera_alt-24px.svg"
            property string addPhotoIconUrl: "qrc:/images/icons/round-add_a_photo-24px.svg"
            property string refreshIconUrl: "qrc:/images/icons/baseline-refresh-24px.svg"

            Layout.preferredWidth: 30
            Layout.preferredHeight: 30
            Layout.alignment: Qt.AlignHCenter

            text: ""
            font.pointSize: 10
            font.kerning: true

            toolTipText: qsTr("Press this button to take photo")

            radius: height / 6
            source: {

                if(takePhotoState) {
                    toolTipText = qsTr("Press this button to finish taking photo")
                    return cameraAltIconUrl
                }

                if(hasAvatar){
                    toolTipText = qsTr("Press this button to retake photo")
                    return refreshIconUrl
                } else {
                    toolTipText = qsTr("Press this button to take photo")
                    return addPhotoIconUrl
                }
            }
            onClicked: {
                if(!takePhotoState){
                    imageCleared()
                    startBooth()
                    return
                } else {
                    // show flash overlay
                    flashOverlay.visible = true
                    flashAnimation.restart()

                    // run concurrent function call to take photo
                    imgBase64 = previewWidget.takeCroppedPhotoToBase64(boothWidth)
                    hasAvatar = true
                    imageAcquired()
                    stopBooth()
                }
            }
        }

        HoverableButton {
            id: importButton

            Layout.preferredWidth: 30
            Layout.preferredHeight: 30
            Layout.alignment: Qt.AlignHCenter

            text: ""
            font.pointSize: 10
            font.kerning: true

            radius: height / 6
            source: "qrc:/images/icons/round-folder-24px.svg"

            toolTipText: qsTr("Import avatar from image file")

            onClicked: {
                importFromFileToAvatar_Dialog.open()
            }
        }
    }
}
