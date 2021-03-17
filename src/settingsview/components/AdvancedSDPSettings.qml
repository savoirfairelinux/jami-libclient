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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Dialogs 1.3
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    function updateSDPAccountInfos(){
        audioRTPMinPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Audio_AudioPortMin())
        audioRTPMaxPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Audio_AudioPortMax())
        videoRTPMinPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Video_VideoPortMin())
        videoRTPMaxPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Video_VideoPortMax())
    }

    function audioRTPMinPortSpinBoxEditFinished(value) {
        if (SettingsAdapter.getAccountConfig_Audio_AudioPortMax() < value) {
            audioRTPMinPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Audio_AudioPortMin())
            return
        }
       SettingsAdapter.audioRTPMinPortSpinBoxEditFinished(value)
    }

    function audioRTPMaxPortSpinBoxEditFinished(value) {
        if (value <SettingsAdapter.getAccountConfig_Audio_AudioPortMin()) {
            audioRTPMaxPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Audio_AudioPortMax())
            return
        }
       SettingsAdapter.audioRTPMaxPortSpinBoxEditFinished(value)
    }

    function videoRTPMinPortSpinBoxEditFinished(value) {
        if (SettingsAdapter.getAccountConfig_Video_VideoPortMax() < value) {
            videoRTPMinPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Video_VideoPortMin())
            return
        }
       SettingsAdapter.videoRTPMinPortSpinBoxEditFinished(value)
    }

    function videoRTPMaxPortSpinBoxEditFinished(value) {
        if (value <SettingsAdapter.getAccountConfig_Video_VideoPortMin()) {
            videoRTPMinPortSpinBox.setValue(SettingsAdapter.getAccountConfig_Video_VideoPortMin())
            return
        }
       SettingsAdapter.videoRTPMaxPortSpinBoxEditFinished(value)
    }

    ElidedTextLabel {
        Layout.preferredWidth: textWidth
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        eText: JamiStrings.sdpSettingsTitle
        fontSize: JamiTheme.headerFontSize
        maxWidth: root.width
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        ElidedTextLabel {
            Layout.preferredWidth: textWidth
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            eText: JamiStrings.sdpSettingsSubtitle
            fontSize: JamiTheme.settingsFontSize
            maxWidth: parent.width - JamiTheme.preferredMarginSize
        }

        SettingSpinBox {
            id: audioRTPMinPortSpinBox

            title: JamiStrings.audioRTPMinPort
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535
            step: 1

            onNewValue: audioRTPMinPortSpinBoxEditFinished(valueField)
        }

        SettingSpinBox {
            id: audioRTPMaxPortSpinBox

            title: JamiStrings.audioRTPMaxPort
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535
            step: 1

            onNewValue: audioRTPMaxPortSpinBoxEditFinished(valueField)
        }

        SettingSpinBox {
            id: videoRTPMinPortSpinBox

            title: JamiStrings.videoRTPMinPort
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535
            step: 1

            onNewValue: videoRTPMinPortSpinBoxEditFinished(valueField)
        }

        SettingSpinBox {
            id: videoRTPMaxPortSpinBox

            title: JamiStrings.videoRTPMaxPort
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 65535
            step: 1

            onNewValue: videoRTPMaxPortSpinBoxEditFinished(valueField)
        }
    }
}
