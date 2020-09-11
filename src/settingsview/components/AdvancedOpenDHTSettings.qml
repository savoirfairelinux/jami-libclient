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

    property int itemWidth

    function updateOpenDHTSettingsInfos() {
        checkAutoConnectOnLocalNetwork.checked = SettingsAdapter.getAccountConfig_PeerDiscovery()
        checkBoxEnableProxy.checked = SettingsAdapter.getAccountConfig_ProxyEnabled()
        lineEditProxy.setText(SettingsAdapter.getAccountConfig_ProxyServer())
        lineEditBootstrap.setText(SettingsAdapter.getAccountConfig_Hostname())
    }

    Text {
        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize / 2

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

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

            onSwitchToggled: {
                SettingsAdapter.setAutoConnectOnLocalNetwork(checked)
            }
        }

        ToggleSwitch {
            id: checkBoxEnableProxy

            labelText: JamiStrings.enableProxy
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setEnableProxy(checked)
                lineEditProxy.setEnabled(checked)
            }
        }

        SettingsMaterialLineEdit {
            id: lineEditProxy

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.proxyAddress

            onEditFinished: SettingsAdapter.setProxyAddress(textField)
        }

        SettingsMaterialLineEdit {
            id: lineEditBootstrap

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.bootstrap

            onEditFinished: SettingsAdapter.setBootstrapAddress(textField)
        }
    }
}