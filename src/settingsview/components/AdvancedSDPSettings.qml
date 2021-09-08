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

import QtQuick
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

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
            topValue: audioRTPMaxPortSpinBox.valueField - 1

            valueField: CurrentAccount.audioPortMin_Audio

            onInputAcceptableChanged: {
                if (!inputAcceptable && valueField.length !== 0)
                    valueField = Qt.binding(function() { return CurrentAccount.audioPortMin_Audio })
            }

            onNewValue: CurrentAccount.audioPortMin_Audio = valueField
        }

        SettingSpinBox {
            id: audioRTPMaxPortSpinBox

            title: JamiStrings.audioRTPMaxPort
            itemWidth: root.itemWidth
            bottomValue: audioRTPMinPortSpinBox.valueField + 1
            topValue: 65535

            valueField: CurrentAccount.audioPortMax_Audio

            onInputAcceptableChanged: {
                if (!inputAcceptable && valueField.length !== 0)
                    valueField = Qt.binding(function() { return CurrentAccount.audioPortMax_Audio })
            }

            onNewValue: CurrentAccount.audioPortMax_Audio = valueField
        }

        SettingSpinBox {
            id: videoRTPMinPortSpinBox

            title: JamiStrings.videoRTPMinPort
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: videoRTPMaxPortSpinBox.valueField - 1

            valueField: CurrentAccount.videoPortMin_Video

            onInputAcceptableChanged: {
                if (!inputAcceptable && valueField.length !== 0)
                    valueField = Qt.binding(function() { return CurrentAccount.videoPortMin_Video })
            }

            onNewValue: CurrentAccount.videoPortMin_Video = valueField
        }

        SettingSpinBox {
            id: videoRTPMaxPortSpinBox

            title: JamiStrings.videoRTPMaxPort
            itemWidth: root.itemWidth
            bottomValue: videoRTPMinPortSpinBox.valueField + 1
            topValue: 65535

            valueField: CurrentAccount.videoPortMax_Video

            onInputAcceptableChanged: {
                if (!inputAcceptable && valueField.length !== 0)
                    valueField = Qt.binding(function() { return CurrentAccount.videoPortMax_Video })
            }

            onNewValue: CurrentAccount.videoPortMax_Video = valueField
        }
    }
}
