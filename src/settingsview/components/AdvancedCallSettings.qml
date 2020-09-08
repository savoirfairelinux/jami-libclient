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
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import Qt.labs.platform 1.1
import "../../commoncomponents"
import "../../constant"

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
            btnRingtone.setText(qsTr("Add a custom ringtone"))
        }
    }

    JamiFileDialog {
        id: ringtonePath_Dialog

        property string oldPath : SettingsAdapter.getAccountConfig_Ringtone_RingtonePath()
        property string openPath : oldPath === "" ? (UtilsAdapter.getCurrentPath() + "/ringtones/") : (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: qsTr("Select a new ringtone")
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

        eText: qsTr("Call Settings")
        fontSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ToggleSwitch {
            id: checkBoxUntrusted
            visible: !root.isSIP

            labelText: qsTr("Allow incoming calls from unknown contacts")
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setCallsUntrusted(checked)
            }
        }

        ToggleSwitch {
            id: checkBoxAutoAnswer

            labelText: qsTr("Auto Answer Calls")
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setAutoAnswerCalls(checked)
            }
        }

        ToggleSwitch {
            id: checkBoxCustomRingtone

            labelText: qsTr("Enable Custom Ringtone")
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setEnableRingtone(checked)
                btnRingtone.setEnabled(checked)
            }
        }

        SettingMaterialButton {
            id: btnRingtone
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            titleField: qsTr("Select Custom Ringtone")
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: ringtonePath_Dialog.open()
        }

        ToggleSwitch {
            id: checkBoxRdv
            visible: !isSIP

            labelText: qsTr("(Experimental) Rendez-vous: turn your account into a conference room")
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setIsRendezVous(checked)
            }
        }
    }
}