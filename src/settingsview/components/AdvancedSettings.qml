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

    property bool isSIP
    property int itemWidth

    signal scrolled

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

            text: qsTr("Advanced Account Settings")
            elide: Text.ElideRight
        }

        HoverableButtonTextItem {
            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.alignment: Qt.AlignHCenter

            radius: height / 2

            toolTipText: JamiStrings.tipAdvancedSettingsDisplay

            source: {
                if (advancedSettingsView.visible) {
                    return "qrc:/images/icons/round-arrow_drop_up-24px.svg"
                } else {
                    return "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                }
            }

            onClicked: {
                advancedSettingsView.visible = !advancedSettingsView.visible
                if(advancedSettingsView.visible)
                    updateAdvancedAccountInfos()
                scrolled()
            }
        }
    }

    ColumnLayout {
        id: advancedSettingsView
        visible: false

        AdvancedCallSettings {
            id: advancedCallSettings
            isSIP: root.isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedVoiceMailSettings {
            id: advancedVoiceMailSettings
            visible: root.isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedSIPSecuritySettings {
            id: advancedSIPSecuritySettings
            visible: root.isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedNameServerSettings {
            id: advancedNameServerSettings
            visible: !root.isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedOpenDHTSettings {
            id: advancedOpenDHTSettings
            visible: !root.isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedJamiSecuritySettings {
            id: advancedJamiSecuritySettings
            visible: !root.isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedConnectivitySettings {
            id: advancedConnectivitySettings
            itemWidth: root.itemWidth
            isSIP: root.isSIP

            Layout.fillWidth: true
        }

        AdvancedPublicAddressSettings {
            id: advancedPublicAddressSettings
            visible: isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }

        AdvancedMediaSettings {
            id: advancedMediaSettings

            Layout.fillWidth: true
        }

        AdvancedSDPSettings {
            id: advancedSDPStettings
            visible: isSIP
            itemWidth: root.itemWidth

            Layout.fillWidth: true
        }
    }
}