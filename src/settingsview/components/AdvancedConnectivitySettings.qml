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

import QtQuick
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth
    property bool isSIP

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

            checked: CurrentAccount.keepAliveEnabled

            onSwitchToggled: CurrentAccount.keepAliveEnabled = checked
        }

        SettingSpinBox {
            id: registrationExpireTimeoutSpinBox

            visible: isSIP

            title: JamiStrings.registrationExpirationTime
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 7*24*3600

            valueField: CurrentAccount.expire_Registration

            onNewValue: CurrentAccount.expire_Registration = valueField
        }

        SettingSpinBox {
            id: networkInterfaceSpinBox

            visible: isSIP

            title: JamiStrings.networkInterface
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535

            valueField: CurrentAccount.localPort

            onInputAcceptableChanged: {
                if (!inputAcceptable && valueField.length !== 0)
                    valueField = Qt.binding(function() { return CurrentAccount.localPort })
            }

            onNewValue: CurrentAccount.localPort = valueField
        }

        ToggleSwitch {
            id: checkBoxUPnP

            Layout.fillWidth: true

            labelText: JamiStrings.useUPnP
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.upnpEnabled

            onSwitchToggled: CurrentAccount.upnpEnabled = checked
        }

        ToggleSwitch {
            id: checkBoxTurnEnable

            Layout.fillWidth: true

            labelText: JamiStrings.useTURN
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.enable_TURN

            onSwitchToggled: CurrentAccount.enable_TURN = checked
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnAddress

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxTurnEnable.checked

            textField: CurrentAccount.server_TURN

            itemWidth: root.itemWidth
            titleField: JamiStrings.turnAdress

            onEditFinished: CurrentAccount.server_TURN = textField
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnUsername

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxTurnEnable.checked

            textField: CurrentAccount.username_TURN

            itemWidth: root.itemWidth
            titleField: JamiStrings.turnUsername

            onEditFinished: CurrentAccount.username_TURN = textField
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnPassword

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxTurnEnable.checked

            textField: CurrentAccount.password_TURN

            itemWidth: root.itemWidth
            titleField: JamiStrings.turnPassword

            onEditFinished: CurrentAccount.password_TURN = textField
        }

        SettingsMaterialLineEdit {
            id: lineEditTurnRealmSIP

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxTurnEnable.checked

            textField: CurrentAccount.realm_TURN

            itemWidth: root.itemWidth
            titleField: JamiStrings.turnRealm

            onEditFinished: CurrentAccount.realm_TURN = textField
        }

        ToggleSwitch {
            id: checkBoxSTUNEnable

            Layout.fillWidth: true

            labelText: JamiStrings.useSTUN
            fontPointSize: JamiTheme.settingsFontSize

            visible: isSIP
            checked: CurrentAccount.enable_STUN

            onSwitchToggled: CurrentAccount.enable_STUN = checked
        }

        SettingsMaterialLineEdit {
            id: lineEditSTUNAddress

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxSTUNEnable.checked
            visible: isSIP

            textField: CurrentAccount.server_STUN

            itemWidth: root.itemWidth
            titleField: JamiStrings.stunAdress

            onEditFinished: CurrentAccount.server_STUN = textField
        }
    }
}
