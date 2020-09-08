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
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import Qt.labs.platform 1.1
import net.jami.Enums 1.0
import "../../commoncomponents"

ColumnLayout {
    id: root

    //TODO: complete check for update and check for Beta slot functions
    function checkForUpdateSlot() {}
    function installBetaSlot() {}

    Label {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        text: qsTr("Updates")
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    ToggleSwitch {
        id: autoUpdateCheckBox

        checked: SettingsAdapter.getAppValue(Settings.Key.AutoUpdate)
        labelText: qsTr("Check for updates automatically")
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: qsTr("toggle automatic updates")

        onSwitchToggled: SettingsAdapter.setAppValue(Settings.Key.AutoUpdate, checked)
    }

    HoverableRadiusButton {
        id: checkUpdateButton

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: JamiTheme.preferredFieldWidth
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        radius: height / 2

        toolTipText: qsTr("Check for updates now")
        text: qsTr("Updates")
        fontPointSize: JamiTheme.buttonFontSize

        onClicked: {
            checkForUpdateSlot()
        }
    }

    HoverableRadiusButton {
        id: installBetaButton

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: JamiTheme.preferredFieldWidth
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        radius: height / 2

        toolTipText: qsTr("Install the latest beta version")
        text: qsTr("Beta Install")
        fontPointSize: JamiTheme.buttonFontSize

        onClicked: {
            installBetaSlot()
        }
    }
}