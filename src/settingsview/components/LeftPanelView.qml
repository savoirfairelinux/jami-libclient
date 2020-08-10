/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
import QtQuick.Window 2.14
import QtQuick.Controls 1.4 as CT
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "../../commoncomponents"
import "../../mainview/components"

Rectangle {
    id: leftPanelRect

    property int contentViewportWidth: 200
    property int contentViewPortHeight: 768

    property alias btnAccountSettings: accountSettingsButton
    property alias btnGeneralSettings: generalSettingsButton
    property alias btnMediaSettings: mediaSettingsButton
    property alias btnPluginSettings: pluginSettingsButton

    signal btnExitClicked

    Component.onCompleted: {
        accountSettingsButton.setCheckedState(true, true)
    }

    anchors.fill: parent
    clip: true
    color: JamiTheme.backgroundColor

    ColumnLayout {
        spacing: 0

        width: contentViewportWidth
        height: contentViewPortHeight

        IconButton {
            id: accountSettingsButton

            buttonText: qsTr("Account")
            imageSource: "qrc:/images/icons/baseline-people-24px.svg"

            onCheckedToggledForLeftPanel: {
                generalSettingsButton.setCheckedState(!checked, false)
                mediaSettingsButton.setCheckedState(!checked, false)
                pluginSettingsButton.setCheckedState(!checked, false)
            }
        }

        IconButton {
            id: generalSettingsButton

            buttonText: qsTr("General")
            imageSource: "qrc:/images/icons/round-settings-24px.svg"

            onCheckedToggledForLeftPanel: {
                accountSettingsButton.setCheckedState(!checked, false)
                mediaSettingsButton.setCheckedState(!checked, false)
                pluginSettingsButton.setCheckedState(!checked, false)
            }
        }

        IconButton {
            id: mediaSettingsButton

            buttonText: qsTr("Audio/Video")
            imageSource: "qrc:/images/icons/baseline-desktop_windows-24px.svg"

            onCheckedToggledForLeftPanel: {
                generalSettingsButton.setCheckedState(!checked, false)
                accountSettingsButton.setCheckedState(!checked, false)
                pluginSettingsButton.setCheckedState(!checked, false)
            }
        }

        IconButton {
            id: pluginSettingsButton

            buttonText: qsTr("Plugins")
            imageSource: "qrc:/images/icons/extension_24dp.svg"

            onCheckedToggledForLeftPanel: {
                generalSettingsButton.setCheckedState(!checked, false)
                accountSettingsButton.setCheckedState(!checked, false)
                mediaSettingsButton.setCheckedState(!checked, false)
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}

