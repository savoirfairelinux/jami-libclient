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
import net.jami.Adapters 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property int preferredColumnWidth: root.width / 2 - 50
    property int preferredSettingsWidth: root.width - 100
    property real aspectRatio: 0.75
    property bool previewAvailable: false

    signal backArrowClicked

    Connections{
        target: AVModel
        enabled: root.visible

        function onAudioMeter(id, level){
            slotAudioMeter(id,level)
        }
    }

    Connections{
        target: RenderManager
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

    function populateAVSettings(){
        audioInputComboBox.model.reset()
        audioOutputDeviceModel.reset()

        audioInputComboBox.currentIndex = audioInputComboBox.model.getCurrentSettingIndex()
        outputComboBox.currentIndex = audioOutputDeviceModel.getCurrentSettingIndex()
        ringtoneDeviceComboBox.currentIndex = audioOutputDeviceModel.getCurrentRingtoneDeviceIndex()

        audioManagerRowLayout.visible = (audioManagerComboBox.model.rowCount() > 0)
        if(audioManagerComboBox.model.rowCount() > 0){
        audioManagerComboBox.currentIndex = audioManagerComboBox.model.getCurrentSettingIndex()
        }

        populateVideoSettings()
        var encodeAccel = AVModel.getHardwareAcceleration()
        hardwareAccelControl.checked = encodeAccel
    }

    function populateVideoSettings() {
        deviceBox.model.reset()

        deviceBox.enabled = (deviceBox.model.deviceCount() > 0)
        resolutionBox.enabled = (deviceBox.model.deviceCount() > 0)
        fpsBox.enabled = (deviceBox.model.deviceCount() > 0)
        labelVideoDevice.enabled = (deviceBox.model.deviceCount() > 0)
        labelVideoResolution.enabled = (deviceBox.model.deviceCount() > 0)
        labelVideoFps.enabled = (deviceBox.model.deviceCount() > 0)

        deviceBox.currentIndex = deviceBox.model.getCurrentSettingIndex()
        slotDeviceBoxCurrentIndexChanged(deviceBox.currentIndex)

        try{
            startPreviewing(false)
        } catch (err2){ console.log("Start preview fail when populate video settings, exception: "+ err2.message)}

    }

    function setFormatListForCurrentDevice(){
        var device = AVModel.getCurrentVideoCaptureDevice()
        if(SettingsAdapter.get_DeviceCapabilitiesSize(device) === 0){
            return
        }

        try{
            resolutionBox.model.reset()
            resolutionBox.currentIndex = resolutionBox.model.getCurrentSettingIndex()
            slotFormatCurrentIndexChanged(resolutionBox.currentIndex,true)
        } catch(err){console.warn("Exception: " + err.message)}
    }

    function startPreviewing(force = false, async = true){
        AccountAdapter.startPreviewing(force, async)
        previewAvailable = true
    }

    function stopPreviewing(async = true){
        AccountAdapter.stopPreviewing(async)
    }

    function startAudioMeter(async = true){
        audioInputMeter.start()
        AccountAdapter.startAudioMeter(async)
    }

    function stopAudioMeter(async = true){
        audioInputMeter.stop()
        AccountAdapter.stopAudioMeter(async)
    }

    // slots for av page
    function slotAudioMeter(id, level){
        if (id === "audiolayer_id") {
                audioInputMeter.setLevel(level)
            }
    }

    function slotSetHardwareAccel(state){
        AVModel.setHardwareAcceleration(state)
        startPreviewing(true)
    }

    function slotAudioManagerIndexChanged(index){
        stopAudioMeter(false)
        var selectedAudioManager = audioManagerComboBox.model.data(audioManagerComboBox.model.index(
                                                        index, 0), AudioManagerListModel.AudioManagerID)
        AVModel.setAudioManager(selectedAudioManager)
        startAudioMeter(false)
    }

    function slotRingtoneDeviceIndexChanged(index){
        stopAudioMeter(false)
        var selectedRingtoneDeviceName = audioOutputDeviceModel.data(audioOutputDeviceModel.index(
                                                        index, 0), AudioOutputDeviceModel.Device_ID)
        AVModel.setRingtoneDevice(selectedRingtoneDeviceName)
        startAudioMeter(false)
    }

    function slotAudioOutputIndexChanged(index){
        stopAudioMeter(false)
        var selectedOutputDeviceName = audioOutputDeviceModel.data(audioOutputDeviceModel.index(
                                                        index, 0), AudioOutputDeviceModel.Device_ID)
        AVModel.setOutputDevice(selectedOutputDeviceName)
        startAudioMeter(false)
    }

    function slotAudioInputIndexChanged(index){
        stopAudioMeter(false)
        var selectedInputDeviceName = audioInputComboBox.model.data(audioInputComboBox.model.index(
                                                        index, 0), AudioInputDeviceModel.Device_ID)

        AVModel.setInputDevice(selectedInputDeviceName)
        startAudioMeter(false)
    }

    function slotDeviceBoxCurrentIndexChanged(index){
        if(deviceBox.model.deviceCount() <= 0){
            return
        }

        try{
            var deviceId = deviceBox.model.data(deviceBox.model.index(
                                                          index, 0), VideoInputDeviceModel.DeviceId)
            var deviceName = deviceBox.model.data(deviceBox.model.index(
                                                            index, 0), VideoInputDeviceModel.DeviceName)
            if(deviceId.length === 0){
                console.warn("Couldn't find device: " + deviceName)
                return
            }

            AVModel.setCurrentVideoCaptureDevice(deviceId)
            AVModel.setDefaultDevice(deviceId)
            setFormatListForCurrentDevice()
            startPreviewing(true)
        } catch(err){console.warn(err.message)}
    }

    function slotFormatCurrentIndexChanged(index, isResolutionIndex){
        var resolution
        var rate
        if(isResolutionIndex){
            resolution = resolutionBox.model.data(resolutionBox.model.index(
                                                                 index, 0), VideoFormatResolutionModel.Resolution)
            fpsBox.model.currentResolution = resolution
            fpsBox.currentIndex = fpsBox.model.getCurrentSettingIndex()
            rate = fpsBox.model.data(fpsBox.model.index(
                                                       fpsBox.currentIndex, 0), VideoFormatFpsModel.FPS)
        } else {
            resolution = resolutionBox.model.data(resolutionBox.model.index(
                                                                 resolutionBox.currentIndex, 0), VideoFormatResolutionModel.Resolution)
            fpsBox.model.currentResolution = resolution
            rate = fpsBox.model.data(fpsBox.model.index(
                                                       index, 0), VideoFormatFpsModel.FPS)
        }

        try{
           SettingsAdapter.set_Video_Settings_Rate_And_Resolution(AVModel.getCurrentVideoCaptureDevice(),rate,resolution)
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

    AudioOutputDeviceModel{
        id: audioOutputDeviceModel
    }

    ColumnLayout {
        anchors.fill: root

        RowLayout {
            id: avSettingsTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.fillWidth: true
            Layout.preferredHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                radius: JamiTheme.preferredFieldHeight
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
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Layout.fillHeight: true
            Layout.fillWidth: true

            focus: true
            clip: true

            ColumnLayout {
                width: root.width

                // Audio
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Audio")
                        fontSize: JamiTheme.headerFontSize
                        maxWidth: width
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            eText: qsTr("Microphone")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }


                        SettingParaCombobox {
                            id: audioInputComboBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            font.pointSize: JamiTheme.buttonFontSize
                            font.kerning: true

                            model: AudioInputDeviceModel {}
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

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: preferredSettingsWidth
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        indeterminate: false
                        from: 0
                        to: 100
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            Layout.fillWidth: true

                            eText: qsTr("Output Device")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }


                        SettingParaCombobox {
                            id: outputComboBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

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
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            Layout.fillWidth: true

                            eText: qsTr("Ringtone Device")
                            font.pointSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }


                        SettingParaCombobox {
                            id: ringtoneDeviceComboBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            font.pointSize: JamiTheme.settingsFontSize
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
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            Layout.fillWidth: true

                            text: qsTr("Audio Manager")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }

                        SettingParaCombobox {
                            id: audioManagerComboBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            font.pointSize: JamiTheme.buttonFontSize
                            font.kerning: true

                            model: AudioManagerListModel {}

                            textRole: "ID_UTF8"

                            onActivated: {
                                slotAudioManagerIndexChanged(index)
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Video")
                        fontSize: JamiTheme.headerFontSize
                        maxWidth: preferredSettingsWidth
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            id: labelVideoDevice

                            Layout.fillWidth: true

                            eText: qsTr("Device")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }


                        SettingParaCombobox {
                            id: deviceBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            font.pointSize: JamiTheme.buttonFontSize
                            font.kerning: true

                            model: VideoInputDeviceModel {}

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
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            id: labelVideoResolution

                            Layout.fillWidth: true

                            eText: qsTr("Resolution")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }

                        SettingParaCombobox {
                            id: resolutionBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            font.pointSize: JamiTheme.buttonFontSize
                            font.kerning: true

                            model: VideoFormatResolutionModel {}
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
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ElidedTextLabel {
                            id: labelVideoFps

                            Layout.fillWidth: true

                            eText: qsTr("Fps")
                            fontSize: JamiTheme.settingsFontSize
                            maxWidth: width
                        }

                        SettingParaCombobox {
                            id: fpsBox

                            Layout.preferredWidth: preferredColumnWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            font.pointSize: JamiTheme.buttonFontSize
                            font.kerning: true

                            model: VideoFormatFpsModel {}
                            textRole: "FPS_ToDisplay_UTF8"

                            tooltipText: qsTr("Video device fps selector")

                            onActivated: {
                                slotFormatCurrentIndexChanged(index,false)
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

                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

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
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize

                    labelText: qsTr("Enable hardware acceleration")
                    fontPointSize: JamiTheme.settingsFontSize

                    onSwitchToggled: {
                        slotSetHardwareAccel(checked)
                    }
                }
            }
        }
    }
}
