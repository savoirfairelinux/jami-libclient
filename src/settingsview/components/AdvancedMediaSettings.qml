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
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    function updateMediaConnectivityAccountInfos() {
        videoCheckBox.checked = SettingsAdapter.getAccountConfig_Video_Enabled()
        videoSettings.updateCodecs();
        audioSettings.updateCodecs();
    }

    Label {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        text: JamiStrings.media
        color: JamiTheme.textColor
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ToggleSwitch {
            id: videoCheckBox

            labelText: JamiStrings.enableVideo
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: SettingsAdapter.setVideoState(checked)
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MediaSettings {
                id: videoSettings

                Layout.fillWidth: true
                Layout.fillHeight: true

                mediaType: MediaSettings.VIDEO
            }

            MediaSettings {
                id: audioSettings

                Layout.fillWidth: true
                Layout.fillHeight: true

                mediaType: MediaSettings.AUDIO
            }
        }
    }
}
