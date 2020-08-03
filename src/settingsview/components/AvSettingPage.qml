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

Rectangle {
    id: avSettingPage

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

    function startPreviewing(force = false){
        ClientWrapper.accountAdaptor.startPreviewing(force)
        previewAvailable = true
    }

    function stopPreviewing(){
        ClientWrapper.accountAdaptor.stopPreviewing()
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
        ClientWrapper.accountAdaptor.avModel().setHardwareAcceleration(state)
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
        } catch(error){console.warn(error.message)}
    }

    function slotVideoDeviceListChanged(){
        populateVideoSettings()
    }

    property bool previewAvailable: false

    Connections{
        target: ClientWrapper.avmodel

        function onAudioMeter(id, level){
            slotAudioMeter(id,level)
        }
    }

    Connections{
        target: ClientWrapper.renderManager

        function onVideoDeviceListChanged(){
            slotVideoDeviceListChanged()
        }
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    ScrollView{
        anchors.fill: parent
        clip: true

        RowLayout {
            width: avSettingPage.width
            height: avSettingPage.height

            spacing: 0
            Item {
                Layout.fillHeight: true
                Layout.maximumWidth: 48
                Layout.preferredWidth: 48
                Layout.minimumWidth: 48
            }

            ColumnLayout {
                spacing: 7

                Layout.fillHeight: true
                Layout.maximumWidth: 580
                Layout.preferredWidth: 580
                Layout.minimumWidth: 580

                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 10
                    Layout.preferredHeight: 10
                    Layout.maximumHeight: 10
                }

                Label {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 25
                    Layout.preferredHeight: 25
                    Layout.maximumHeight: 25

                    text: qsTr("Audio / Video")
                    font.pointSize: 15
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 24
                    Layout.preferredHeight: 24
                    Layout.maximumHeight: 24
                }

                ColumnLayout {
                    spacing: 0
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 21
                        Layout.preferredHeight: 21
                        Layout.maximumHeight: 21

                        text: qsTr("Audio")
                        font.pointSize: 13
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 10
                        Layout.preferredHeight: 10
                        Layout.maximumHeight: 10
                    }

                    ColumnLayout {
                        spacing: 7
                        Layout.fillWidth: true

                        RowLayout {
                            spacing: 7
                            Layout.leftMargin: 20
                            Layout.fillWidth: true
                            Layout.maximumHeight: 30

                            Label {
                                Layout.maximumWidth: 77
                                Layout.preferredWidth: 77
                                Layout.minimumWidth: 77

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Microphone")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }

                            SettingParaCombobox {
                                id: inputComboBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
                                font.kerning: true

                                model: audioInputDeviceModel

                                textRole: "ID_UTF8"

                                onActivated: {
                                    slotAudioInputIndexChanged(index)
                                }
                            }
                        }

                        // the audio level meter
                        LevelMeter {
                            id: audioInputMeter

                            Layout.leftMargin: 20

                            Layout.fillWidth: true

                            Layout.minimumHeight: 10
                            Layout.preferredHeight: 10
                            Layout.maximumHeight: 10

                            indeterminate: false
                            from: 0
                            to: 100
                        }

                        Item {
                            Layout.fillWidth: true

                            Layout.minimumHeight: 5
                            Layout.preferredHeight: 5
                            Layout.maximumHeight: 5
                        }

                        RowLayout {
                            spacing: 7
                            Layout.leftMargin: 20
                            Layout.fillWidth: true
                            Layout.maximumHeight: 30

                            Label {
                                Layout.maximumWidth: 95
                                Layout.preferredWidth: 95
                                Layout.minimumWidth: 95

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Output Device")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }

                            SettingParaCombobox {
                                id: outputComboBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
                                font.kerning: true

                                model: audioOutputDeviceModel

                                textRole: "ID_UTF8"

                                onActivated: {
                                    slotAudioOutputIndexChanged(index)
                                }
                            }
                        }

                        RowLayout {
                            spacing: 7
                            Layout.leftMargin: 20
                            Layout.fillWidth: true
                            Layout.maximumHeight: 30

                            Label {
                                Layout.maximumWidth: 77
                                Layout.preferredWidth: 77
                                Layout.minimumWidth: 77

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Ringtone Device")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }

                            SettingParaCombobox {
                                id: ringtoneDeviceComboBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
                                font.kerning: true

                                model: audioOutputDeviceModel

                                textRole: "ID_UTF8"

                                onActivated: {
                                    slotRingtoneDeviceIndexChanged(index)
                                }
                            }
                        }

                        RowLayout {
                            id: audioManagerRowLayout

                            spacing: 7
                            Layout.leftMargin: 20
                            Layout.fillWidth: true
                            Layout.maximumHeight: 30

                            Label {
                                Layout.maximumWidth: 77
                                Layout.preferredWidth: 77
                                Layout.minimumWidth: 77

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Audio Manager")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }

                            SettingParaCombobox {
                                id: audioManagerComboBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
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

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.maximumHeight: 20
                }

                ColumnLayout {
                    spacing: 7
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 30
                        Layout.preferredHeight: 30
                        Layout.maximumHeight: 30

                        text: qsTr("Video")
                        font.pointSize: 13
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item {
                        Layout.fillWidth: true

                        Layout.minimumHeight: 10
                        Layout.preferredHeight: 10
                        Layout.maximumHeight: 10
                    }

                    ColumnLayout {
                        spacing: 6
                        RowLayout {
                            spacing: 7
                            Layout.fillWidth: true
                            Layout.leftMargin: 20
                            Layout.maximumHeight: 30

                            Label {
                                id: labelVideoDevice

                                Layout.maximumWidth: 44
                                Layout.preferredWidth: 44
                                Layout.minimumWidth: 44

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Device")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }

                            SettingParaCombobox {
                                id: deviceBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
                                font.kerning: true

                                model: videoInputDeviceModel

                                textRole: "DeviceName_UTF8"

                                onActivated: {
                                    slotDeviceBoxCurrentIndexChanged(index)
                                }
                            }
                        }

                        RowLayout {
                            spacing: 7
                            Layout.fillWidth: true
                            Layout.leftMargin: 20
                            Layout.maximumHeight: 30

                            Label {
                                id: labelVideoResolution

                                Layout.maximumWidth: 47
                                Layout.preferredWidth: 47
                                Layout.minimumWidth: 47

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Resolution")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }

                            SettingParaCombobox {
                                id: resolutionBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
                                font.kerning: true

                                model: videoFormatResolutionModel
                                textRole: "Resolution_UTF8"

                                onActivated: {
                                    slotFormatCurrentIndexChanged(index,true)
                                }
                            }
                        }

                        RowLayout {
                            spacing: 7
                            Layout.fillWidth: true
                            Layout.leftMargin: 20
                            Layout.maximumHeight: 30

                            Label {
                                id: labelVideoFps

                                Layout.maximumWidth: 47
                                Layout.preferredWidth: 47
                                Layout.minimumWidth: 47

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                text: qsTr("Fps")
                                font.pointSize: 11
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }

                            SettingParaCombobox {
                                id: fpsBox

                                Layout.maximumWidth: 360
                                Layout.preferredWidth: 360
                                Layout.minimumWidth: 360

                                Layout.minimumHeight: 30
                                Layout.preferredHeight: 30
                                Layout.maximumHeight: 30

                                font.pointSize: 10
                                font.kerning: true

                                model: videoFormatFpsModel
                                textRole: "FPS_ToDisplay_UTF8"

                                onActivated: {
                                    slotFormatCurrentIndexChanged(index,false)
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.maximumHeight: 20
                }

                RowLayout{
                    Layout.alignment: Qt.AlignHCenter

                    Layout.preferredWidth: 580
                    Layout.minimumWidth: 580

                    Layout.minimumHeight: 224
                    Layout.preferredHeight: 224
                    Layout.maximumHeight: 224

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: true
                        Layout.minimumWidth: 580

                        color: "black"

                        PreviewRenderer{
                            id: peviewWidget

                            visible: previewAvailable
                            height: parent.height
                            width: 224
                            x: (parent.width - width) /2
                            y: 0
                        }
                    }
                }

                Label {
                    visible: !previewAvailable

                    Layout.fillWidth: true

                    Layout.minimumHeight: 30
                    Layout.preferredHeight: 30
                    Layout.maximumHeight: 30

                    text: qsTr("Preview unavailable")
                    font.pointSize: 10
                    font.kerning: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                // Toggle switch to enable hardware acceleration
                ToggleSwitch {
                    id: hardwareAccelControl

                    labelText: "Enable hardware acceleration"

                    onSwitchToggled: {
                        slotSetHardwareAccel(checked)
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }

    }
}
