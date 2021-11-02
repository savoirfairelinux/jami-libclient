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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Constants 1.1

ColumnLayout {
    id:root

    property int itemWidth

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

        checked: CurrentAccount.autoTransferFromUntrusted

        labelText: JamiStrings.allowFromUntrusted
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.allowFromUntrusted

        onSwitchToggled: CurrentAccount.autoTransferFromUntrusted = checked
    }

    ToggleSwitch {
        id: autoAcceptFilesCheckbox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: CurrentAccount.autoTransferFromTrusted

        labelText: JamiStrings.autoAcceptFiles
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.autoAcceptFiles

        onSwitchToggled: CurrentAccount.autoTransferFromTrusted = checked
    }

    SettingSpinBox {
        id: acceptTransferBelowSpinBox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        title: JamiStrings.acceptTransferBelow
        tooltipText: JamiStrings.acceptTransferTooltip
        itemWidth: root.itemWidth
        bottomValue: 0

        valueField: CurrentAccount.autoTransferSizeThreshold

        onNewValue: CurrentAccount.autoTransferSizeThreshold = valueField
    }
}
