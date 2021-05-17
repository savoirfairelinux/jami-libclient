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
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth
    property bool isSIP

    function updateConnectivityAccountInfos() {
        autoRegistrationAfterExpired.checked = SettingsAdapter.getAccountConfig_KeepAliveEnabled()
        registrationExpireTimeoutSpinBox.setValue(SettingsAdapter.getAccountConfig_Registration_Expire())
        networkInterfaceSpinBox.setValue(SettingsAdapter.getAccountConfig_Localport())
        checkBoxUPnP.checked = SettingsAdapter.getAccountConfig_UpnpEnabled()
        checkBoxTurnEnable.checked = SettingsAdapter.getAccountConfig_TURN_Enabled()
        lineEditTurnAddress.setText(SettingsAdapter.getAccountConfig_TURN_Server())
        lineEditTurnUsername.setText(SettingsAdapter.getAccountConfig_TURN_Username())
        lineEditTurnPassword.setText(SettingsAdapter.getAccountConfig_TURN_Password())
        checkBoxSTUNEnable.checked = SettingsAdapter.getAccountConfig_STUN_Enabled()
        lineEditSTUNAddress.setText(SettingsAdapter.getAccountConfig_STUN_Server())
        lineEditTurnRealmSIP.setText(SettingsAdapter.getAccountConfig_TURN_Realm())
        lineEditTurnRealmSIP.setEnabled(SettingsAdapter.getAccountConfig_TURN_Enabled())
        lineEditSTUNAddress.setEnabled(SettingsAdapter.getAccountConfig_STUN_Enabled())

    }

    ElidedTextLabel {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        eText: JamiStrings.connectivity
        fontSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ToggleSwitch {
            id: autoRegistrationAfterExpired

            Layout.fillWidth: true

            visible: isSIP
            labelText: JamiStrings.autoRegistration
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: SettingsAdapter.setKeepAliveEnabled(checked)
        }

        SettingSpinBox {
            id: registrationExpireTimeoutSpinBox
            visible: isSIP

            title: JamiStrings.registrationExpirationTime
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 7*24*3600
            step: 1

            onNewValue: SettingsAdapter.registrationExpirationTimeSpinBoxValueChanged(valueField)
        }

        SettingSpinBox {
            id: networkInterfaceSpinBox
            visible: isSIP

            title: JamiStrings.networkInterface
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535
            step: 1

            onNewValue: SettingsAdapter.networkInterfaceSpinBoxValueChanged(valueField)
        }

        ToggleSwitch {
            id: checkBoxUPnP

            Layout.fillWidth: true

            labelText: JamiStrings.useUPnP
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: SettingsAdapter.setUseUPnP(checked)
        }

        ToggleSwitch {
            id: checkBoxTurnEnable

            Layout.fillWidth: true

            labelText: JamiStrings.useTURN
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseTURN(checked)
                if (isSIP) {
                    lineEditTurnAddress.setEnabled(checked)
                    lineEditTurnUsername.setEnabled(checked)
                    lineEditTurnPassword.setEnabled(checked)
                    lineEditTurnRealmSIP.setEnabled(checked)
                }
            }
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnAddress

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.turnAdress
            onEditFinished: SettingsAdapter.setTURNAddress(textField)
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnUsername

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.turnUsername
            onEditFinished: SettingsAdapter.setTURNUsername(textField)
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnPassword

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.turnPassword
            onEditFinished: SettingsAdapter.setTURNPassword(textField)
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnRealmSIP

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.turnRealm
            onEditFinished: SettingsAdapter.setTURNRealm(textField)
        }

        ToggleSwitch {
            id: checkBoxSTUNEnable

            Layout.fillWidth: true

            labelText: JamiStrings.useSTUN
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseSTUN(checked)
                lineEditSTUNAddress.enabled = checked
            }
        }

        SettingsMaterialLineEdit {
            id: lineEditSTUNAddress

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.stunAdress
            onEditFinished: SettingsAdapter.setSTUNAddress(textField)
        }
    }
}
