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

    enum Type {
        VIDEO,
        AUDIO
    }

    property int mediaType

    function decreaseCodecPriority() {
        var index = mediaListWidget.currentIndex
        if (index >= mediaListWidget.model.rowCount() - 1)
            return
        var codecId = mediaListWidget.model.data(mediaListWidget.model.index(index,0),
                                                 MediaCodecListModel.MediaCodecID)

        if (mediaType === MediaSettings.VIDEO)
            SettingsAdapter.decreaseVideoCodecPriority(codecId)
        else if (mediaType === MediaSettings.AUDIO)
            SettingsAdapter.decreaseAudioCodecPriority(codecId)
        mediaListWidget.currentIndex = index + 1
        updateCodecs()
    }

    function updateCodecs() {
        mediaListWidget.model.layoutAboutToBeChanged()
        mediaListWidget.model.dataChanged(mediaListWidget.model.index(0, 0),
                                          mediaListWidget.model.index(
                                              mediaListWidget.model.rowCount() - 1, 0))
        mediaListWidget.model.layoutChanged()
    }

    function increaseCodecPriority(){
        var index = mediaListWidget.currentIndex
        if (index === 0)
            return
        var codecId = mediaListWidget.model.data(mediaListWidget.model.index(index,0),
                                                 MediaCodecListModel.MediaCodecID)

        if (mediaType === MediaSettings.VIDEO)
            SettingsAdapter.increaseVideoCodecPriority(codecId)
        else if (mediaType === MediaSettings.AUDIO)
            SettingsAdapter.increaseAudioCodecPriority(codecId)
        mediaListWidget.currentIndex = index - 1
        updateCodecs()
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.maximumHeight: JamiTheme.preferredFieldHeight

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            maxWidth: width
            eText:  {
                if (mediaType === MediaSettings.VIDEO)
                    return "Video Codecs"
                else if (mediaType === MediaSettings.AUDIO)
                    return "Audio Codecs"
            }
            fontSize: JamiTheme.settingsFontSize
        }

        PushButton {
            source: "qrc:/images/icons/arrow_drop_down-24px.svg"
            onClicked: decreaseCodecPriority()
        }

        PushButton {
            source: "qrc:/images/icons/arrow_drop_up-24px.svg"
            onClicked: increaseCodecPriority()
        }
    }

    ListViewJami {
        id: mediaListWidget

        Layout.fillWidth: true
        Layout.preferredHeight: 190

        model: MediaCodecListModel {
            mediaType: root.mediaType
        }

        delegate: MediaCodecDelegate {
            id: mediaCodecDelegate

            width: mediaListWidget.width
            height: mediaListWidget.height / 4

            mediaCodecName : MediaCodecName
            isEnabled : IsEnabled
            mediaCodecId: MediaCodecID
            samplerRate: Samplerate
            mediaType: root.mediaType

            onClicked: {
                mediaListWidget.currentIndex = index
            }

            onMediaCodecStateChange: {
                if (mediaType === MediaSettings.VIDEO)
                    SettingsAdapter.videoCodecsStateChange(idToSet, isToBeEnabled)
                if (mediaType === MediaSettings.AUDIO)
                    SettingsAdapter.audioCodecsStateChange(idToSet, isToBeEnabled)
                updateCodecs()
            }
        }
    }
}
