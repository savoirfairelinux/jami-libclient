/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import "../../commoncomponents"

Rectangle {
    id: root

    AudioInputDeviceModel{
        id: audioInputDeviceModel
    }

    AudioOutputDeviceModel{
        id: audioOutputDeviceModel
    }

    AudioManagerListModel{
        id: audioManagerListModel
    }

    VideoInputDeviceModel{
        id: videoInputDeviceModel
    }

    VideoFormatResolutionModel{
        id: videoFormatResolutionModel
    }

    VideoFormatFpsModel{
        id: videoFormatFpsModel
    }

    function populateAVSettings(){
        audioInputDeviceModel.reset()
        audioOutputDeviceModel.reset()

        inputComboBox.currentIndex = audioInputDeviceModel.getCurrentSettingIndex()
        outputComboBox.currentIndex = audioOutputDeviceModel.getCurrentSettingIndex()
        ringtoneDeviceComboBox.currentIndex = audioOutputDeviceModel.getCurrentRingtoneDeviceIndex()

        audioManagerRowLayout.visible = (audioManagerListModel.rowCount() > 0)
        if(audioManagerListModel.rowCount() > 0){
        audioManagerComboBox.currentIndex = audioManagerListModel.getCurrentSettingIndex()
        }

        populateVideoSettings()
        var encodeAccel = ClientWrapper.avmodel.getHardwareAcceleration()
        hardwareAccelControl.checked = encodeAccel
    }

    function populateVideoSettings() {
        videoInputDeviceModel.reset()

        deviceBox.enabled = (videoInputDeviceModel.deviceCount() > 0)
        resolutionBox.enabled = (videoInputDeviceModel.deviceCount() > 0)
        fpsBox.enabled = (videoInputDeviceModel.deviceCount() > 0)
        labelVideoDevice.enabled = (videoInputDeviceModel.deviceCount() > 0)
        labelVideoResolution.enabled = (videoInputDeviceModel.deviceCount() > 0)
        labelVideoFps.enabled = (videoInputDeviceModel.deviceCount() > 0)

        deviceBox.currentIndex = videoInputDeviceModel.getCurrentSettingIndex()
        slotDeviceBoxCurrentIndexChanged(deviceBox.currentIndex)

        try{
            startPreviewing(false)
        } catch (err2){ console.log("Start preview fail when populate video settings, exception: "+ err2.message)}

    }

    function setFormatListForCurrentDevice(){
        var device = ClientWrapper.avmodel.getCurrentVideoCaptureDevice()
        if(ClientWrapper.settingsAdaptor.get_DeviceCapabilitiesSize(device) === 0){
            return
        }

        try{
            videoFormatResolutionModel.reset()
            resolutionBox.currentIndex = videoFormatResolutionModel.getCurrentSettingIndex()
            slotFormatCurrentIndexChanged(resolutionBox.currentIndex,true)
        } catch(err){console.warn("Exception: " + err.message)}
    }

    function startPreviewing(force = false, async = true){
        ClientWrapper.accountAdaptor.startPreviewing(force, async)
        previewAvailable = true
    }

    function stopPreviewing(async = true){
        ClientWrapper.accountAdaptor.stopPreviewing(async)
    }

    function startAudioMeter(async = true){
        audioInputMeter.start()
        ClientWrapper.accountAdaptor.startAudioMeter(async)
    }

    function stopAudioMeter(async = true){
        audioInputMeter.stop()
        ClientWrapper.accountAdaptor.stopAudioMeter(async)
    }

    // slots for av page
    function slotAudioMeter(id, level){
        if (id === "audiolayer_id") {
                audioInputMeter.setLevel(level)
            }
    }

    function slotSetHardwareAccel(state){
        ClientWrapper.avmodel.setHardwareAcceleration(state)
        startPreviewing(true)
    }

    function slotAudioManagerIndexChanged(index){
        stopAudioMeter(false)
        var selectedAudioManager = audioManagerListModel.data(audioManagerListModel.index(
                                                        index, 0), AudioManagerListModel.AudioManagerID)
        ClientWrapper.avmodel.setAudioManager(selectedAudioManager)
        startAudioMeter(false)
    }

    function slotRingtoneDeviceIndexChanged(index){
        stopAudioMeter(false)
        var selectedRingtoneDeviceName = audioOutputDeviceModel.data(audioOutputDeviceModel.index(
                                                        index, 0), AudioOutputDeviceModel.Device_ID)
        ClientWrapper.avmodel.setRingtoneDevice(selectedRingtoneDeviceName)
        startAudioMeter(false)
    }

    function slotAudioOutputIndexChanged(index){
        stopAudioMeter(false)
        var selectedOutputDeviceName = audioOutputDeviceModel.data(audioOutputDeviceModel.index(
                                                        index, 0), AudioOutputDeviceModel.Device_ID)
        ClientWrapper.avmodel.setOutputDevice(selectedOutputDeviceName)
        startAudioMeter(false)
    }

    function slotAudioInputIndexChanged(index){
        stopAudioMeter(false)
        var selectedInputDeviceName = audioInputDeviceModel.data(audioInputDeviceModel.index(
                                                        index, 0), AudioInputDeviceModel.Device_ID)

        ClientWrapper.avmodel.setInputDevice(selectedInputDeviceName)
        startAudioMeter(false)
    }

    function slotDeviceBoxCurrentIndexChanged(index){
        if(videoInputDeviceModel.deviceCount() <= 0){
            return
        }

        try{
            var deviceId = videoInputDeviceModel.data(videoInputDeviceModel.index(
                                                          index, 0), VideoInputDeviceModel.DeviceId)
            var deviceName = videoInputDeviceModel.data(videoInputDeviceModel.index(
                                                            index, 0), VideoInputDeviceModel.DeviceName)
            if(deviceId.length === 0){
                console.warn("Couldn't find device: " + deviceName)
                return
            }

            ClientWrapper.avmodel.setCurrentVideoCaptureDevice(deviceId)
            ClientWrapper.avmodel.setDefaultDevice(deviceId)
            setFormatListForCurrentDevice()
            startPreviewing(true)
        } catch(err){console.warn(err.message)}
    }

    function slotFormatCurrentIndexChanged(index, isResolutionIndex){
        var resolution
        var rate
        if(isResolutionIndex){
            resolution = videoFormatResolutionModel.data(videoFormatResolutionModel.index(
                                                                 index, 0), VideoFormatResolutionModel.Resolution)
            videoFormatFpsModel.currentResolution = resolution
            fpsBox.currentIndex = videoFormatFpsModel.getCurrentSettingIndex()
            rate = videoFormatFpsModel.data(videoFormatFpsModel.index(
                                                       fpsBox.currentIndex, 0), VideoFormatFpsModel.FPS)
        } else {
            resolution = videoFormatResolutionModel.data(videoFormatResolutionModel.index(
                                                                 resolutionBox.currentIndex, 0), VideoFormatResolutionModel.Resolution)
            videoFormatFpsModel.currentResolution = resolution
            rate = videoFormatFpsModel.data(videoFormatFpsModel.index(
                                                       index, 0), VideoFormatFpsModel.FPS)
        }

        try{
            ClientWrapper.settingsAdaptor.set_Video_Settings_Rate_And_Resolution(ClientWrapper.avmodel.getCurrentVideoCaptureDevice(),rate,resolution)
            updatePreviewRatio(resolution)
        } catch(error){console.warn(error.message)}
    }

    function slotVideoDeviceListChanged(){
        populateVideoSettings()
    }

    function updatePreviewRatio(resolution){
        var res = resolution.split("x")
        var ratio = res[1] / res[0]
        if (ratio) {
            aspectRatio = ratio
        } else {
            console.error("Could not scale recording video preview")
        }
    }

    property int preferredColumnWidth: avSettingsScrollView.width / 2 - 50
    property int preferredSettingsWidth: avSettingsScrollView.width - 100

    property real aspectRatio: 0.75

    property bool previewAvailable: false
    signal backArrowClicked

    Connections{
        target: ClientWrapper.avmodel
        enabled: root.visible

        function onAudioMeter(id, level){
            slotAudioMeter(id,level)
        }
    }

    Connections{
        target: ClientWrapper.renderManager
        enabled: root.visible

        function onVideoDeviceListChanged(){
            slotVideoDeviceListChanged()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            stopPreviewing()
            stopAudioMeter()
        }
    }

    Layout.fillHeight: true
    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView
    anchors.centerIn: parent

    ColumnLayout {
        anchors.fill: root

        RowLayout {
            id: avSettingsTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.fillWidth: true
            Layout.maximumHeight: 64
            Layout.minimumHeight: 64
            Layout.preferredHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.rightMargin: JamiTheme.preferredMarginSize

                radius: 30
                source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                backgroundColor: "white"
                onExitColor: "white"
                toolTipText: qsTr("Toggle to display side panel")
                hoverEnabled: true

                visible: mainViewWindow.sidePanelHidden

                onClicked: {
                    backArrowClicked()
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                text: qsTr("Audio / Video")
                font.pointSize: JamiTheme.titleFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

        }


        ScrollView {
            id: avSettingsScrollView
            property ScrollBar vScrollBar: ScrollBar.vertical

            Layout.fillHeight: true
            Layout.fillWidth: true

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            width: root.width
            height: root.height - avSettingsTitle.height

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            clip: true

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                spacing: 24

                // Audio
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Audio")
                        fontSize: JamiTheme.headerFontSize
                        maxWidth: preferredColumnWidth
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {
                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Microphone")
                                fontSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }


                            SettingParaCombobox {
                                id: inputComboBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.buttonFontSize
                                font.kerning: true

                                model: audioInputDeviceModel
                                textRole: "ID_UTF8"
                                tooltipText: qsTr("Audio input device selector")
                                onActivated: {
                                    slotAudioInputIndexChanged(index)
                                }
                            }
                        }

                        // the audio level meter
                        LevelMeter {
                            id: audioInputMeter

                            Layout.minimumWidth: preferredSettingsWidth
                            Layout.preferredWidth: preferredSettingsWidth
                            Layout.maximumWidth: preferredSettingsWidth
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            indeterminate: false
                            from: 0
                            to: 100
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {
                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Output Device")
                                fontSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }


                            SettingParaCombobox {
                                id: outputComboBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.settingsFontSize
                                font.kerning: true

                                model: audioOutputDeviceModel
                                textRole: "ID_UTF8"
                                tooltipText: qsTr("Choose the audio output device")
                                onActivated: {
                                    slotAudioOutputIndexChanged(index)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {

                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Ringtone Device")
                                font.pointSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }


                            SettingParaCombobox {
                                id: ringtoneDeviceComboBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.buttonFontSize
                                font.kerning: true

                                model: audioOutputDeviceModel

                                textRole: "ID_UTF8"
                                tooltipText: qsTr("Choose the ringtone output device")
                                onActivated: {
                                    slotRingtoneDeviceIndexChanged(index)
                                }
                            }
                        }

                        RowLayout {
                            id: audioManagerRowLayout

                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {
                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                text: qsTr("Audio Manager")
                                fontSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }

                            SettingParaCombobox {
                                id: audioManagerComboBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.buttonFontSize
                                font.kerning: true

                                model: audioManagerListModel

                                textRole: "ID_UTF8"

                                onActivated: {
                                    slotAudioManagerIndexChanged(index)
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Video")
                        fontSize: JamiTheme.headerFontSize
                        maxWidth: preferredSettingsWidth
                    }

                    ColumnLayout {
                        Layout.leftMargin: 16

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {
                                id: labelVideoDevice

                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Device")
                                fontSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }


                            SettingParaCombobox {
                                id: deviceBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.buttonFontSize
                                font.kerning: true

                                model: videoInputDeviceModel

                                textRole: "DeviceName_UTF8"
                                tooltipText: qsTr("Video device selector")
                                onActivated: {
                                    slotDeviceBoxCurrentIndexChanged(index)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {
                                id: labelVideoResolution

                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Resolution")
                                fontSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }

                            SettingParaCombobox {
                                id: resolutionBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.buttonFontSize
                                font.kerning: true

                                model: videoFormatResolutionModel
                                textRole: "Resolution_UTF8"

                                tooltipText: qsTr("Video device resolution selector")

                                onActivated: {
                                    slotFormatCurrentIndexChanged(index,true)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {
                                id: labelVideoFps

                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Fps")
                                fontSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }

                            SettingParaCombobox {
                                id: fpsBox

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                font.pointSize: JamiTheme.buttonFontSize
                                font.kerning: true

                                model: videoFormatFpsModel
                                textRole: "FPS_ToDisplay_UTF8"

                                tooltipText: qsTr("Video device fps selector")

                                onActivated: {
                                    slotFormatCurrentIndexChanged(index,false)
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: rectBox
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumHeight: width * aspectRatio
                    Layout.minimumHeight: width * aspectRatio
                    Layout.preferredHeight: width * aspectRatio

                    Layout.minimumWidth: 200
                    Layout.maximumWidth: 400
                    Layout.preferredWidth: preferredSettingsWidth
                    color: "white"
                    radius: 5

                    PreviewRenderer {
                        id: previewWidget
                        anchors.fill: rectBox
                        anchors.centerIn: rectBox

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: rectBox
                        }
                    }
                }


                Label {
                    visible: !previewAvailable

                    Layout.fillWidth: true

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    text: qsTr("Preview unavailable")
                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                // Toggle switch to enable hardware acceleration
                ToggleSwitch {
                    id: hardwareAccelControl

                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    labelText: hwAccelText.elidedText
                    fontPointSize: JamiTheme.settingsFontSize

                    onSwitchToggled: {
                        slotSetHardwareAccel(checked)
                    }
                }

                TextMetrics {
                    id: hwAccelText
                    elide: Text.ElideRight
                    elideWidth: preferredSettingsWidth - 50
                    text: qsTr("Enable hardware acceleration")
                }

                Item {
                    Layout.preferredWidth: root.width - 32
                    Layout.minimumWidth: root.width - 32
                    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView - 32
                    Layout.fillHeight: true
                }
            }
        }
    }
}
