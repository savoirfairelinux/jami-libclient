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

import "../../commoncomponents"
import "../../constant"

ColumnLayout {
    id: root

    property int itemWidth

    function updatePublicAddressAccountInfos() {
        checkBoxCustomAddressPort.checked = SettingsAdapter.getAccountConfig_PublishedSameAsLocal()
        lineEditSIPCustomAddress.setText(SettingsAdapter.getAccountConfig_PublishedAddress())
        customPortSIPSpinBox.setValue(SettingsAdapter.getAccountConfig_PublishedPort())
    }

    Text {
        Layout.fillWidth: true

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        text: JamiStrings.publicAddress
        color: JamiTheme.textColor
        elide: Text.ElideRight
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ToggleSwitch {
            id: checkBoxCustomAddressPort

            labelText: JamiStrings.useCustomAddress
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseCustomAddressAndPort(checked)
                lineEditSIPCustomAddress.setEnabled(checked)
                customPortSIPSpinBox.setEnabled(checked)
            }
        }

        SettingsMaterialLineEdit {
            id: lineEditSIPCustomAddress

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.address

            onEditFinished: SettingsAdapter.lineEditSIPCustomAddressLineEditTextChanged(textField)
        }

        SettingSpinBox {
            id: customPortSIPSpinBox

            title: JamiStrings.port
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535
            step: 1

            onNewValue: SettingsAdapter.customPortSIPSpinBoxValueChanged(valueField)
        }
    }
}
