/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property real aspectRatio: 0.75
    property bool previewAvailable: false
    property int itemWidth

    Connections {
        target: AvAdapter

        function onVideoDeviceListChanged() {
            populateVideoSettings()
        }
    }

    function startPreviewing(force = false) {
        if (root.visible) {
            AccountAdapter.startPreviewing(force)
            previewAvailable = true
        }
    }

    function populateVideoSettings() {
        deviceComboBoxSetting.comboModel.reset()

        var count = deviceComboBoxSetting.comboModel.deviceCount()

        previewWidget.visible = count > 0
        deviceComboBoxSetting.enabled = count > 0
        resolutionComboBoxSetting.enabled = count > 0
        fpsComboBoxSetting.enabled = count > 0

        if (count === 0) {
            resolutionComboBoxSetting.reset()
            fpsComboBoxSetting.reset()
        } else {
            deviceComboBoxSetting.modelIndex =
                    deviceComboBoxSetting.comboModel.getCurrentIndex()
        }
        hardwareAccelControl.checked = AVModel.getHardwareAcceleration()
    }

    function slotDeviceBoxCurrentIndexChanged(index) {
        if (deviceComboBoxSetting.comboModel.deviceCount() <= 0)
            return

        try {
            var deviceId = deviceComboBoxSetting.comboModel.data(
                        deviceComboBoxSetting.comboModel.index(index, 0),
                        VideoInputDeviceModel.DeviceId)
            var deviceName = deviceComboBoxSetting.comboModel.data(
                        deviceComboBoxSetting.comboModel.index(index, 0),
                        VideoInputDeviceModel.DeviceName)
            if(deviceId.length === 0) {
                console.warn("Couldn't find device: " + deviceName)
                return
            }

            if (AVModel.getCurrentVideoCaptureDevice() !== deviceId) {
                AVModel.setCurrentVideoCaptureDevice(deviceId)
                AVModel.setDefaultDevice(deviceId)
            }

            resolutionComboBoxSetting.reset()
        } catch(err){ console.warn(err.message) }
    }

    function updatePreviewRatio(resolution) {
        var res = resolution.split("x")
        var ratio = res[1] / res[0]
        if (ratio) {
            aspectRatio = ratio
        } else {
            console.error("Could not scale recording video preview")
        }
    }

    onVisibleChanged: {
        if (visible)
            startPreviewing(true)
    }

    ElidedTextLabel {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        eText: JamiStrings.video
        fontSize: JamiTheme.headerFontSize
        maxWidth: itemWidth * 2
    }

    SettingsComboBox {
        id: deviceComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.device
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: VideoInputDeviceModel {
            lrcInstance: LRCInstance
        }
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectVideoDevice
        role: "DeviceName_UTF8"

        onModelIndexChanged: slotDeviceBoxCurrentIndexChanged(modelIndex)

        placeholderText: JamiStrings.noVideoDevice
    }

    SettingsComboBox {
        id: resolutionComboBoxSetting

        function reset() {
            modelIndex = -1
            comboModel.reset()
            modelIndex = 0
        }

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.resolution
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: VideoFormatResolutionModel {
            lrcInstance: LRCInstance
        }
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectVideoResolution
        role: "Resolution_UTF8"

        modelIndex: -1

        onModelIndexChanged: {
            if (modelIndex === -1)
                return
            var resolution = comboModel.data(comboModel.index(modelIndex, 0),
                                             VideoFormatResolutionModel.Resolution)
            fpsComboBoxSetting.comboModel.currentResolution = resolution
            fpsComboBoxSetting.modelIndex = 0

            var rate = fpsComboBoxSetting.comboModel.data(
                        fpsComboBoxSetting.comboModel.index(0, 0),
                        VideoFormatFpsModel.FPS)

            AvAdapter.setCurrentVideoDeviceRateAndResolution(rate, resolution)
            updatePreviewRatio(resolution)
        }
    }

    SettingsComboBox {
        id: fpsComboBoxSetting

        function reset() {
            modelIndex = -1
            comboModel.reset()
            modelIndex = 0
        }

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.fps
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: VideoFormatFpsModel {
            lrcInstance: LRCInstance
        }
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectFPS
        role: "FPS_ToDisplay_UTF8"

        modelIndex: -1

        onModelIndexChanged: {
            if (modelIndex === -1)
                return
            var resolution = resolutionComboBoxSetting.comboModel.data(
                        resolutionComboBoxSetting.comboModel.index(
                            resolutionComboBoxSetting.modelIndex, 0),
                        VideoFormatResolutionModel.Resolution)

            var rate = comboModel.data(comboModel.index(modelIndex, 0),
                                       VideoFormatFpsModel.FPS)

            AvAdapter.setCurrentVideoDeviceRateAndResolution(rate, resolution)
        }
    }

    ToggleSwitch {
        id: hardwareAccelControl

        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.enableHWAccel
        fontPointSize: JamiTheme.settingsFontSize

        onSwitchToggled: {
            AVModel.setHardwareAcceleration(checked)
            startPreviewing(true)
        }
    }

    // video Preview
    Rectangle {
        id: rectBox

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: width * aspectRatio

        Layout.minimumWidth: 200
        Layout.maximumWidth: 400
        Layout.preferredWidth: itemWidth * 2
        Layout.bottomMargin: JamiTheme.preferredMarginSize

        color: "white"
        radius: 5

        PreviewRenderer {
            id: previewWidget

            anchors.fill: rectBox

            lrcInstance: LRCInstance

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
        Layout.bottomMargin: JamiTheme.preferredMarginSize

        text: JamiStrings.previewUnavailable
        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
