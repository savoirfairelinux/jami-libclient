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
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Enums 1.0
import Qt.labs.platform 1.1
import "../../commoncomponents"

ColumnLayout {
    id:root

    property int itemWidth

    enum Setting {
        AUDIOINPUT,
        AUDIOOUTPUT,
        RINGTONEDEVICE,
        AUDIOMANAGER
    }

    Connections {
        target: settingsViewRect

        function onStopAudioMeter() {
            stopAudioMeter()
        }

        function onStartAudioMeter() {
            startAudioMeter()
        }
    }

    Connections{
        target: AVModel
        enabled: root.visible

        function onAudioMeter(id, level) {
            if (id === "audiolayer_id") {
                audioInputMeter.setLevel(level)
            }
        }
    }

    function populateAudioSettings() {
        inputComboBoxSetting.comboModel.reset()
        audioOutputDeviceModel.reset()
        audioManagerComboBoxSetting.comboModel.reset()

        inputComboBoxSetting.setCurrentIndex(inputComboBoxSetting.comboModel.getCurrentSettingIndex())
        outputComboBoxSetting.setCurrentIndex(audioOutputDeviceModel.getCurrentSettingIndex())
        ringtoneDeviceComboBoxSetting.setCurrentIndex(audioOutputDeviceModel.getCurrentRingtoneDeviceIndex())
        if(audioManagerComboBoxSetting.comboModel.rowCount() > 0) {
            audioManagerComboBoxSetting.setCurrentIndex(audioManagerComboBoxSetting.comboModel.getCurrentSettingIndex())
        }

        audioManagerComboBoxSetting.visible = (audioManagerComboBoxSetting.comboModel.rowCount() > 0)
    }

    function stopAudioMeter(async = true){
        audioInputMeter.stop()
        AccountAdapter.stopAudioMeter(async)
    }

    function startAudioMeter(async = true){
        audioInputMeter.start()
        AccountAdapter.startAudioMeter(async)
    }

    AudioOutputDeviceModel{
        id: audioOutputDeviceModel
    }

    ElidedTextLabel {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        eText: qsTr("Audio")
        fontSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    SettingsComboBox {
        id: inputComboBoxSetting

        Layout.fillWidth: true
        Layout.maximumHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: qsTr("Microphone")
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: AudioInputDeviceModel {}
        widthOfComboBox: itemWidth
        tipText: qsTr("Audio input device selector")
        role: "ID_UTF8"

        onIndexChanged: {
            stopAudioMeter(false)
            var selectedInputDeviceName = comboModel.data(comboModel.index( modelIndex, 0), AudioInputDeviceModel.Device_ID)
            AVModel.setInputDevice(selectedInputDeviceName)
            startAudioMeter(false)
        }
    }

    // the audio level meter
    LevelMeter {
        id: audioInputMeter

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: itemWidth * 2
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        indeterminate: false
        from: 0
        to: 100
    }

    SettingsComboBox {
        id: outputComboBoxSetting

        Layout.fillWidth: true
        Layout.maximumHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: qsTr("Output Device")
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: audioOutputDeviceModel
        widthOfComboBox: itemWidth
        tipText: qsTr("Choose the audio output device")
        role: "ID_UTF8"

        onIndexChanged: {
            stopAudioMeter(false)
            var selectedOutputDeviceName = audioOutputDeviceModel.data(audioOutputDeviceModel.index( modelIndex, 0), AudioOutputDeviceModel.Device_ID)
            AVModel.setOutputDevice(selectedOutputDeviceName)
            startAudioMeter(false)
        }
    }

    SettingsComboBox {
        id: ringtoneDeviceComboBoxSetting

        Layout.fillWidth: true
        Layout.maximumHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: qsTr("Ringtone Device")
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: audioOutputDeviceModel
        widthOfComboBox: itemWidth
        tipText: qsTr("Choose the ringtone output device")
        role: "ID_UTF8"

        onIndexChanged: {
            stopAudioMeter(false)
            var selectedRingtoneDeviceName = audioOutputDeviceModel.data(audioOutputDeviceModel.index( modelIndex, 0), AudioOutputDeviceModel.Device_ID)
            AVModel.setRingtoneDevice(selectedRingtoneDeviceName)
            startAudioMeter(false)
        }
    }

    SettingsComboBox {
        id: audioManagerComboBoxSetting

        Layout.fillWidth: true
        Layout.maximumHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: qsTr("Audio Manager")
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: AudioManagerListModel {}
        widthOfComboBox: itemWidth
        role: "ID_UTF8"

        onIndexChanged: {
            stopAudioMeter(false)
            var selectedAudioManager = comboModel.data(comboModel.index( modelIndex, 0), AudioManagerListModel.AudioManagerID)
            AVModel.setAudioManager(selectedAudioManager)
            startAudioMeter(false)
            populateAudioSettings()
        }
    }
}
