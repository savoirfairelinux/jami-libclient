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
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import Qt.labs.platform 1.1
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    property bool isSIP
    property int itemWidth

    function updateCallSettingsInfos() {
        checkBoxUntrusted.checked = SettingsAdapter.getAccountConfig_DHT_PublicInCalls()
        checkBoxRdv.checked = SettingsAdapter.getAccountConfig_RendezVous()
        checkBoxAutoAnswer.checked = SettingsAdapter.getAccountConfig_AutoAnswer()
        checkBoxCustomRingtone.checked = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()

        btnRingtone.setEnabled(SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled())
        btnRingtone.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_Ringtone_RingtonePath()))
    }

    function changeRingtonePath(url) {
        if(url.length !== 0) {
           SettingsAdapter.set_RingtonePath(url)
            btnRingtone.setText(UtilsAdapter.toFileInfoName(url))
        } else if (SettingsAdapter.getAccountConfig_Ringtone_RingtonePath().length === 0){
            btnRingtone.setText(JamiStrings.addCustomRingtone)
        }
    }

    JamiFileDialog {
        id: ringtonePath_Dialog

        property string oldPath : SettingsAdapter.getAccountConfig_Ringtone_RingtonePath()
        property string openPath : oldPath === "" ? (UtilsAdapter.getCurrentPath() + "/ringtones/") : (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectNewRingtone
        folder: openPath

        nameFilters: [qsTr("Audio Files") + " (*.wav *.ogg *.opus *.mp3 *.aiff *.wma)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())
            changeRingtonePath(url)
        }
    }

    ElidedTextLabel {
        Layout.fillWidth: true

        eText: JamiStrings.callSettings
        fontSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ToggleSwitch {
            id: checkBoxUntrusted
            visible: !root.isSIP

            labelText: JamiStrings.allowCallsUnknownContacs
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setCallsUntrusted(checked)
            }
        }

        ToggleSwitch {
            id: checkBoxAutoAnswer

            labelText: JamiStrings.autoAnswerCalls
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setAutoAnswerCalls(checked)
            }
        }

        ToggleSwitch {
            id: checkBoxCustomRingtone

            labelText: JamiStrings.enableCustomRingtone
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setEnableRingtone(checked)
                btnRingtone.setEnabled(checked)
            }
        }

        SettingMaterialButton {
            id: btnRingtone
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.selectCustomRingtone
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: ringtonePath_Dialog.open()
        }

        ToggleSwitch {
            id: checkBoxRdv
            visible: !isSIP

            labelText: JamiStrings.rendezVous
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setIsRendezVous(checked)
            }
        }
    }
}
