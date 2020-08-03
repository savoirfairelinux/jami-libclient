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

import "../../commoncomponents"

ScrollView{
    id: leftPanelView

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

    ColumnLayout {
        spacing: 0

        width: contentViewportWidth
        height: contentViewPortHeight

        Item {
            Layout.fillWidth: true
            Layout.maximumHeight: 13
            Layout.preferredHeight: 13
            Layout.minimumHeight: 13
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 20
            Layout.preferredHeight: 20
            Layout.minimumHeight: 20

            Layout.rightMargin: 14

            Item {
                Layout.fillHeight: true

                Layout.maximumWidth: 20
                Layout.preferredWidth: 20
                Layout.minimumWidth: 20
            }

            Label {
                Layout.maximumWidth: 57
                Layout.preferredWidth: 57
                Layout.minimumWidth: 57

                Layout.maximumHeight: 30
                Layout.preferredHeight: 30
                Layout.minimumHeight: 30

                text: qsTr("Settings")
                font.pointSize: 12
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            HoverableRadiusButton {
                id: btnExitSettings
                Layout.maximumWidth: 30
                Layout.preferredWidth: 30
                Layout.minimumWidth: 30

                Layout.maximumHeight: 30
                Layout.preferredHeight: 30
                Layout.minimumHeight: 30

                buttonImageHeight: height
                buttonImageWidth: height
                backgroundColor: "transparent"

                radius: height / 2

                icon.source: "qrc:/images/icons/round-close-24px.svg"
                icon.height: 24
                icon.width: 24

                onClicked: {
                    btnExitClicked()
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.maximumHeight: 13
            Layout.preferredHeight: 13
            Layout.minimumHeight: 13
        }

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

