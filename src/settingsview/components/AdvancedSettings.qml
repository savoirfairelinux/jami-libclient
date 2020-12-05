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
import net.jami.Constants 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    property bool isSIP
    property int itemWidth
    property alias settingsVisible: advancedSettingsView.visible

    function updateAdvancedAccountInfos() {
        advancedCallSettings.updateCallSettingsInfos()
        advancedVoiceMailSettings.updateVoiceMailSettingsInfos()
        advancedSIPSecuritySettings.updateSecurityAccountInfos()
        advancedNameServerSettings.updateNameServerInfos()
        advancedOpenDHTSettings.updateOpenDHTSettingsInfos()
        advancedJamiSecuritySettings.updateSecurityAccountInfos()
        advancedConnectivitySettings.updateConnectivityAccountInfos()
        advancedPublicAddressSettings.updatePublicAddressAccountInfos()
        advancedMediaSettings.updateMediaConnectivityAccountInfos()
        advancedSDPStettings.updateSDPAccountInfos()
    }

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
            toolTipText: JamiStrings.tipAdvancedSettingsDisplay

            preferredSize: 32
            source: {
                if (advancedSettingsView.visible) {
                    return "qrc:/images/icons/expand_less-24px.svg"
                } else {
                    return "qrc:/images/icons/expand_more-24px.svg"
                }
            }

            onClicked: {
                advancedSettingsView.visible = !advancedSettingsView.visible
                if(advancedSettingsView.visible)
                    updateAdvancedAccountInfos()
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

            isSIP: root.isSIP
            itemWidth: root.itemWidth
        }

        AdvancedVoiceMailSettings {
            id: advancedVoiceMailSettings

            Layout.fillWidth: true

            visible: root.isSIP
            itemWidth: root.itemWidth
        }

        AdvancedSIPSecuritySettings {
            id: advancedSIPSecuritySettings

            Layout.fillWidth: true

            visible: root.isSIP
            itemWidth: root.itemWidth
        }

        AdvancedNameServerSettings {
            id: advancedNameServerSettings

            Layout.fillWidth: true

            visible: !root.isSIP
            itemWidth: root.itemWidth
        }

        AdvancedOpenDHTSettings {
            id: advancedOpenDHTSettings

            Layout.fillWidth: true

            visible: !root.isSIP
            itemWidth: root.itemWidth
        }

        AdvancedJamiSecuritySettings {
            id: advancedJamiSecuritySettings

            Layout.fillWidth: true

            visible: !root.isSIP
            itemWidth: root.itemWidth
        }

        AdvancedConnectivitySettings {
            id: advancedConnectivitySettings

            Layout.fillWidth: true

            itemWidth: root.itemWidth
            isSIP: root.isSIP
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
