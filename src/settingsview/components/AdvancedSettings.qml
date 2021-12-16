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

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property bool isSIP
    property int itemWidth
    property alias settingsVisible: advancedSettingsView.visible
    signal showAdvancedSettingsRequest

    RowLayout {
        id: rowAdvancedSettingsBtn
        Layout.fillWidth: true
        Layout.bottomMargin: 8

        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            font.pointSize: JamiTheme.headerFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: JamiTheme.textColor

            text: qsTr("Advanced Account Settings")
            elide: Text.ElideRight
        }

        PushButton {
            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.alignment: Qt.AlignHCenter

            imageColor: JamiTheme.textColor
            toolTipText: advancedSettingsView.visible ?
                            JamiStrings.tipAdvancedSettingsHide :
                            JamiStrings.tipAdvancedSettingsDisplay

            preferredSize: 32
            source: advancedSettingsView.visible ?
                        JamiResources.expand_less_24dp_svg :
                        JamiResources.expand_more_24dp_svg

            onClicked: {
                advancedSettingsView.visible = !advancedSettingsView.visible
                showAdvancedSettingsRequest()
            }
        }
    }

    ColumnLayout {
        id: advancedSettingsView

        Layout.fillWidth: true

        visible: false

        AdvancedCallSettings {
            id: advancedCallSettings

            Layout.fillWidth: true

            isSIP: LRCInstance.currentAccountType === Profile.Type.SIP
            itemWidth: root.itemWidth
        }

        AdvancedChatSettings {
            id: advancedChatSettings

            Layout.fillWidth: true

            visible: LRCInstance.currentAccountType === Profile.Type.JAMI
            itemWidth: root.itemWidth
        }

        AdvancedVoiceMailSettings {
            id: advancedVoiceMailSettings

            Layout.fillWidth: true

            visible: LRCInstance.currentAccountType === Profile.Type.SIP
            itemWidth: root.itemWidth
        }

        AdvancedSIPSecuritySettings {
            id: advancedSIPSecuritySettings

            Layout.fillWidth: true

            visible: LRCInstance.currentAccountType === Profile.Type.SIP
            itemWidth: root.itemWidth
        }

        AdvancedNameServerSettings {
            id: advancedNameServerSettings

            Layout.fillWidth: true

            visible: LRCInstance.currentAccountType === Profile.Type.JAMI
            itemWidth: root.itemWidth
        }

        AdvancedOpenDHTSettings {
            id: advancedOpenDHTSettings

            Layout.fillWidth: true

            visible: LRCInstance.currentAccountType === Profile.Type.JAMI
            itemWidth: root.itemWidth
        }

        AdvancedJamiSecuritySettings {
            id: advancedJamiSecuritySettings

            Layout.fillWidth: true

            visible: LRCInstance.currentAccountType === Profile.Type.JAMI
            itemWidth: root.itemWidth
        }

        AdvancedConnectivitySettings {
            id: advancedConnectivitySettings

            Layout.fillWidth: true

            itemWidth: root.itemWidth
            isSIP: LRCInstance.currentAccountType === Profile.Type.SIP
        }

        AdvancedPublicAddressSettings {
            id: advancedPublicAddressSettings

            Layout.fillWidth: true

            visible: isSIP
            itemWidth: root.itemWidth
        }

        AdvancedMediaSettings {
            id: advancedMediaSettings

            Layout.fillWidth: true
        }

        AdvancedSDPSettings {
            id: advancedSDPStettings

            Layout.fillWidth: true

            visible: isSIP
            itemWidth: root.itemWidth
        }
    }
}
