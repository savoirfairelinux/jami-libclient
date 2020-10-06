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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import Qt.labs.platform 1.1
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Enums 1.0

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

    AudioOutputDeviceModel{
        id: audioOutputDeviceModel
    }

    ElidedTextLabel {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        eText: JamiStrings.audio
        fontSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    SettingsComboBox {
        id: inputComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.microphone
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: AudioInputDeviceModel {}
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectAudioInputDevice
        role: "ID_UTF8"

        onIndexChanged: {
            AvAdapter.stopAudioMeter(false)
            var selectedInputDeviceName = comboModel.data(
                        comboModel.index(modelIndex, 0),
                        AudioInputDeviceModel.Device_ID)
            AVModel.setInputDevice(selectedInputDeviceName)
            AvAdapter.startAudioMeter(false)
        }
    }

    // the audio level meter
    LevelMeter {
        id: audioInputMeter

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: itemWidth * 1.5
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        indeterminate: false
        from: 0
        to: 100
    }

    SettingsComboBox {
        id: outputComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.outputDevice
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: audioOutputDeviceModel
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectAudioOutputDevice
        role: "ID_UTF8"

        onIndexChanged: {
            AvAdapter.stopAudioMeter(false)
            var selectedOutputDeviceName = audioOutputDeviceModel.data(
                        audioOutputDeviceModel.index(modelIndex, 0),
                        AudioOutputDeviceModel.Device_ID)
            AVModel.setOutputDevice(selectedOutputDeviceName)
            AvAdapter.startAudioMeter(false)
        }
    }

    SettingsComboBox {
        id: ringtoneDeviceComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.ringtoneDevice
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: audioOutputDeviceModel
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectRingtoneOutputDevice
        role: "ID_UTF8"

        onIndexChanged: {
            AvAdapter.stopAudioMeter(false)
            var selectedRingtoneDeviceName = audioOutputDeviceModel.data(
                        audioOutputDeviceModel.index(modelIndex, 0),
                        AudioOutputDeviceModel.Device_ID)
            AVModel.setRingtoneDevice(selectedRingtoneDeviceName)
            AvAdapter.startAudioMeter(false)
        }
    }

    SettingsComboBox {
        id: audioManagerComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.audioManager
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: AudioManagerListModel {}
        widthOfComboBox: itemWidth
        role: "ID_UTF8"

        onIndexChanged: {
            AvAdapter.stopAudioMeter(false)
            var selectedAudioManager = comboModel.data(
                        comboModel.index(modelIndex, 0), AudioManagerListModel.AudioManagerID)
            AVModel.setAudioManager(selectedAudioManager)
            AvAdapter.startAudioMeter(false)
            populateAudioSettings()
        }
    }
}
