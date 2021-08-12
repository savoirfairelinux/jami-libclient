/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 * Author: Albert Babí Oller <albert.babi@savoirfairelinux.com>
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
import net.jami.Constants 1.1

import "../../commoncomponents"
import "../../mainview/components"
import "../../mainview/js/contactpickercreation.js" as ContactPickerCreation

ColumnLayout {
    id: root

    property bool isSIP
    property int itemWidth

    function updateCallSettingsInfos() {
        checkBoxUntrusted.checked = SettingsAdapter.getAccountConfig_DHT_PublicInCalls()
        checkBoxRdv.checked = SettingsAdapter.getAccountConfig_RendezVous()
        checkBoxAutoAnswer.checked = SettingsAdapter.getAccountConfig_AutoAnswer()
        checkBoxCustomRingtone.checked = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()
        checkboxAllModerators.checked = SettingsAdapter.isAllModeratorsEnabled(LRCInstance.currentAccountId)

        btnRingtone.enabled = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()
        btnRingtone.textField = UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_Ringtone_RingtonePath())
        updateAndShowModeratorsSlot()
    }

    function changeRingtonePath(url) {
        if(url.length !== 0) {
           SettingsAdapter.set_RingtonePath(url)
            btnRingtone.textField = UtilsAdapter.toFileInfoName(url)
        } else if (SettingsAdapter.getAccountConfig_Ringtone_RingtonePath().length === 0){
            btnRingtone.textField = JamiStrings.addCustomRingtone
        }
    }

    function updateAndShowModeratorsSlot() {
        toggleLocalModerators.checked = SettingsAdapter.isLocalModeratorsEnabled(
                    LRCInstance.currentAccountId)
        moderatorListWidget.model.reset()
        moderatorListWidget.visible = (moderatorListWidget.model.rowCount() > 0)
    }

    Connections {
        target: ContactAdapter

        function onDefaultModeratorsUpdated() {
            updateAndShowModeratorsSlot()
        }
    }

    JamiFileDialog {
        id: ringtonePath_Dialog

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectNewRingtone
        folder: JamiQmlUtils.qmlFilePrefix + UtilsAdapter.toFileAbsolutepath(
                    SettingsAdapter.getAccountConfig_Ringtone_RingtonePath())

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
                btnRingtone.enabled = checked
            }
        }

        SettingMaterialButton {
            id: btnRingtone
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.selectCustomRingtone
            source: JamiResources.round_folder_24dp_svg
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

        ToggleSwitch {
            id: toggleLocalModerators

            labelText: JamiStrings.enableLocalModerators
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: SettingsAdapter.enableLocalModerators(
                                 LRCInstance.currentAccountId, checked)
        }

        ElidedTextLabel {
            Layout.fillWidth: true

            eText: JamiStrings.defaultModerators
            fontSize: JamiTheme.settingsFontSize
            maxWidth: root.width - JamiTheme.preferredFieldHeight
                        - JamiTheme.preferredMarginSize * 4
        }

        ListViewJami {
            id: moderatorListWidget

            Layout.fillWidth: true
            Layout.preferredHeight: 160

            model: ModeratorListModel {
                lrcInstance: LRCInstance
            }

            delegate: ContactItemDelegate {
                id: moderatorListDelegate

                width: moderatorListWidget.width
                height: 74

                contactName: ContactName
                contactID: ContactID

                btnImgSource: JamiResources.round_remove_circle_24dp_svg
                btnToolTip: JamiStrings.removeDefaultModerator

                onClicked: moderatorListWidget.currentIndex = index
                onBtnContactClicked: {
                    SettingsAdapter.setDefaultModerator(
                                LRCInstance.currentAccountId, contactID, false)
                    updateAndShowModeratorsSlot()
                }
            }
        }

        MaterialButton {
            id: addDefaultModeratorPushButton

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.preferredFieldWidth
            preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true
            toolTipText: JamiStrings.addDefaultModerator

            iconSource: JamiResources.round_add_24dp_svg

            text: JamiStrings.addDefaultModerator

            onClicked: {
                ContactPickerCreation.createContactPickerObjects(
                            ContactList.CONVERSATION,
                            mainView)
                ContactPickerCreation.openContactPicker()
            }
        }

        ToggleSwitch {
            id: checkboxAllModerators

            labelText: JamiStrings.enableAllModerators
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: SettingsAdapter.setAllModeratorsEnabled(
                                 LRCInstance.currentAccountId, checked)
        }
    }
}
