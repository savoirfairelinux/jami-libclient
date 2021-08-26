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
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Constants 1.1

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
        inputComboBoxSetting.modelIndex = inputComboBoxSetting.comboModel.getCurrentIndex()
        outputComboBoxSetting.modelIndex = outputComboBoxSetting.comboModel.getCurrentIndex()
        ringtoneComboBoxSetting.modelIndex = outputComboBoxSetting.comboModel.getCurrentIndex()
        if(audioManagerComboBoxSetting.comboModel.rowCount() > 0) {
            audioManagerComboBoxSetting.modelIndex =
                    audioManagerComboBoxSetting.comboModel.getCurrentSettingIndex()
        }
        audioManagerComboBoxSetting.visible = audioManagerComboBoxSetting.comboModel.rowCount() > 0
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
        comboModel: AudioDeviceModel {
            lrcInstance: LRCInstance
            type: AudioDeviceModel.Type.Record
        }
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectAudioInputDevice
        role: "DeviceName"

        onModelIndexChanged: {
            AvAdapter.stopAudioMeter()
            AVModel.setInputDevice(comboModel.data(
                                       comboModel.index(modelIndex, 0),
                                       AudioDeviceModel.RawDeviceName))
            AvAdapter.startAudioMeter()
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
        comboModel: AudioDeviceModel {
            lrcInstance: LRCInstance
            type: AudioDeviceModel.Type.Playback
        }
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectAudioOutputDevice
        role: "DeviceName"

        onModelIndexChanged: {
            AvAdapter.stopAudioMeter()
            AVModel.setOutputDevice(comboModel.data(
                                        comboModel.index(modelIndex, 0),
                                        AudioDeviceModel.RawDeviceName))
            AvAdapter.startAudioMeter()
        }
    }

    SettingsComboBox {
        id: ringtoneComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.ringtoneDevice
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: AudioDeviceModel {
            lrcInstance: LRCInstance
            type: AudioDeviceModel.Type.Ringtone
        }
        widthOfComboBox: itemWidth
        tipText: JamiStrings.selectRingtoneOutputDevice
        role: "DeviceName"

        onModelIndexChanged: {
            AvAdapter.stopAudioMeter()
            AVModel.setRingtoneDevice(comboModel.data(
                                          comboModel.index(modelIndex, 0),
                                          AudioDeviceModel.RawDeviceName))
            AvAdapter.startAudioMeter()
        }
    }

    SettingsComboBox {
        id: audioManagerComboBoxSetting

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        labelText: JamiStrings.audioManager
        fontPointSize: JamiTheme.settingsFontSize
        comboModel: AudioManagerListModel {
            lrcInstance: LRCInstance
        }
        widthOfComboBox: itemWidth
        role: "ID_UTF8"

        onModelIndexChanged: {
            AvAdapter.stopAudioMeter()
            var selectedAudioManager = comboModel.data(
                        comboModel.index(modelIndex, 0), AudioManagerListModel.AudioManagerID)
            AVModel.setAudioManager(selectedAudioManager)
            AvAdapter.startAudioMeter()
            populateAudioSettings()
        }
    }
}
