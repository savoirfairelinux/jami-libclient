/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import Qt.labs.platform 1.1

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id:root

    property int itemWidth

    Label {
        Layout.fillWidth: true

        text: JamiStrings.chatview
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
        color: JamiTheme.textColor

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    ToggleSwitch {
        id: enableTypingIndicatorCheckbox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: SettingsAdapter.getAppValue(Settings.EnableTypingIndicator)

        labelText: JamiStrings.enableTypingIndicator
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.enableTypingIndicator

        onSwitchToggled: SettingsAdapter.setAppValue(Settings.Key.EnableTypingIndicator, checked)
    }

    ToggleSwitch {
        id: displayImagesCheckbox
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: SettingsAdapter.getAppValue(Settings.DisplayHyperlinkPreviews)

        labelText: JamiStrings.displayHyperlinkPreviews
        fontPointSize: JamiTheme.settingsFontSize

        tooltipText: JamiStrings.displayHyperlinkPreviews

        onSwitchToggled: {
            SettingsAdapter.setAppValue(Settings.Key.DisplayHyperlinkPreviews, checked)
            MessagesAdapter.setDisplayLinks()
        }
    }
}
