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
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Enums 1.0
import Qt.labs.platform 1.1

import "../../commoncomponents"

ColumnLayout {
    id:root

    property int itemWidth
    property string downloadPath: SettingsAdapter.getDir_Download()

    onDownloadPathChanged: {
        if(downloadPath === "") return
       SettingsAdapter.setDownloadPath(downloadPath)
    }

    FolderDialog {
        id: downloadPathDialog

        title: JamiStrings.selectFolder
        currentFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

        onAccepted: {
            var dir = UtilsAdapter.getAbsPath(folder.toString())
            downloadPath = dir
        }
    }

    Label {
        Layout.fillWidth: true

        text: qsTr("System")
        color: JamiTheme.textColor
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    ToggleSwitch {
        id: notificationCheckBox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: SettingsAdapter.getAppValue(Settings.EnableNotifications)

        labelText: qsTr("Enable desktop notifications")
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.enableNotifications

        onSwitchToggled: SettingsAdapter.setAppValue(Settings.Key.EnableNotifications, checked)
    }

    ToggleSwitch {
        id: closeOrMinCheckBox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize
        checked: SettingsAdapter.getAppValue(Settings.MinimizeOnClose)

        labelText: JamiStrings.keepMinimized
        fontPointSize: JamiTheme.settingsFontSize

        onSwitchToggled: SettingsAdapter.setAppValue(Settings.Key.MinimizeOnClose, checked)
    }

    ToggleSwitch {
        id: applicationOnStartUpCheckBox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: UtilsAdapter.checkStartupLink()

        labelText: JamiStrings.runStartup
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.tipRunStartup

        onSwitchToggled: SettingsAdapter.setRunOnStartUp(checked)
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: JamiTheme.textColor
            text: JamiStrings.downloadFolder
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        MaterialButton {
            id: downloadButton

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: itemWidth
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            toolTipText: JamiStrings.tipChooseDownloadFolder
            text: downloadPath
            source: "qrc:/images/icons/round-folder-24px.svg"
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: downloadPathDialog.open()
        }
    }
}
