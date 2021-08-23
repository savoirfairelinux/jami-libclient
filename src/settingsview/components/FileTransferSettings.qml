/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtQuick.Layouts 1.14

import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Constants 1.1

ColumnLayout {
    id:root

    property int itemWidth

    function updateValues() {
        acceptTransferBelowSpinBox.setValue(SettingsAdapter.getAppValue(Settings.AcceptTransferBelow))
        allowFromUntrustedCheckbox.checked = SettingsAdapter.getAppValue(Settings.AllowFromUntrusted)
        autoAcceptFilesCheckbox.checked = SettingsAdapter.getAppValue(Settings.AutoAcceptFiles)
    }

    Label {
        Layout.fillWidth: true

        text: JamiStrings.fileTransfer
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
        color: JamiTheme.textColor

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    ToggleSwitch {
        id: allowFromUntrustedCheckbox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: SettingsAdapter.getAppValue(Settings.AllowFromUntrusted)

        labelText: JamiStrings.allowFromUntrusted
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.allowFromUntrusted

        onSwitchToggled: SettingsAdapter.allowFromUntrusted(checked)
    }

    ToggleSwitch {
        id: autoAcceptFilesCheckbox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: SettingsAdapter.getAppValue(Settings.AutoAcceptFiles)

        labelText: JamiStrings.autoAcceptFiles
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.autoAcceptFiles

        onSwitchToggled: SettingsAdapter.autoAcceptFiles(checked)
    }

    SettingSpinBox {
        id: acceptTransferBelowSpinBox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        title: JamiStrings.acceptTransferBelow
        tooltipText: JamiStrings.acceptTransferTooltip
        itemWidth: root.itemWidth
        bottomValue: 0
        topValue: 99999999
        step: 1

        onNewValue: SettingsAdapter.acceptTransferBelow(valueField)
    }
}
