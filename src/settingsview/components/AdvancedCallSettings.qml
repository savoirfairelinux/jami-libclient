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

    function updateAndShowModeratorsSlot() {
        moderatorListWidget.model.reset()
        moderatorListWidget.visible = moderatorListWidget.model.rowCount() > 0
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
                    CurrentAccount.ringtonePath_Ringtone)

        nameFilters: [qsTr("Audio Files") + " (*.wav *.ogg *.opus *.mp3 *.aiff *.wma)",
            qsTr("All files") + " (*)"]

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())

            if(url.length !== 0)
                CurrentAccount.ringtonePath_Ringtone = url
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

            checked: CurrentAccount.PublicInCalls_DHT

            onSwitchToggled: CurrentAccount.PublicInCalls_DHT = checked
        }

        ToggleSwitch {
            id: checkBoxAutoAnswer

            labelText: JamiStrings.autoAnswerCalls
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.autoAnswer

            onSwitchToggled: CurrentAccount.autoAnswer = checked
        }

        ToggleSwitch {
            id: checkBoxCustomRingtone

            labelText: JamiStrings.enableCustomRingtone
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.ringtoneEnabled_Ringtone

            onSwitchToggled: CurrentAccount.ringtoneEnabled_Ringtone = checked
        }

        SettingMaterialButton {
            id: btnRingtone

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxCustomRingtone.checked

            textField: UtilsAdapter.toFileInfoName(CurrentAccount.ringtonePath_Ringtone)

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

            checked: CurrentAccount.isRendezVous

            onSwitchToggled: CurrentAccount.isRendezVous = checked
        }

        ToggleSwitch {
            id: toggleLocalModerators

            labelText: JamiStrings.enableLocalModerators
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.isLocalModeratorsEnabled

            onSwitchToggled: CurrentAccount.isLocalModeratorsEnabled = checked
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

            visible: model.rowCount() > 0

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
                    AccountAdapter.setDefaultModerator(
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

            checked: CurrentAccount.isAllModeratorsEnabled

            onSwitchToggled: CurrentAccount.isAllModeratorsEnabled = checked
        }
    }
}
