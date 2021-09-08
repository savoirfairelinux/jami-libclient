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

    Text {
        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize / 2

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        color: JamiTheme.textColor
        text: JamiStrings.openDHTConfig
        elide: Text.ElideRight
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ToggleSwitch {
            id: checkAutoConnectOnLocalNetwork
            visible: !root.isSIP

            Layout.fillWidth: true

            labelText: JamiStrings.enablePeerDiscovery
            tooltipText: JamiStrings.tooltipPeerDiscovery
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.peerDiscovery

            onSwitchToggled: CurrentAccount.peerDiscovery = checked
        }

        ToggleSwitch {
            id: checkBoxEnableProxy

            labelText: JamiStrings.enableProxy
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.proxyEnabled

            onSwitchToggled: CurrentAccount.proxyEnabled = checked
        }

        SettingsMaterialLineEdit {
            id: lineEditProxy

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: checkBoxEnableProxy.checked

            textField: CurrentAccount.proxyServer

            itemWidth: root.itemWidth
            titleField: JamiStrings.proxyAddress

            onEditFinished: CurrentAccount.proxyServer = textField
        }

        SettingsMaterialLineEdit {
            id: lineEditBootstrap

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            textField: CurrentAccount.hostname

            itemWidth: root.itemWidth
            titleField: JamiStrings.bootstrap

            onEditFinished: CurrentAccount.hostname = textField
        }
    }
}
