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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

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

        AvAdapter.decreaseCodecPriority(codecId, mediaType === MediaSettings.VIDEO)
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

        AvAdapter.increaseCodecPriority(codecId, mediaType === MediaSettings.VIDEO)
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
                    return JamiStrings.videoCodecs
                else if (mediaType === MediaSettings.AUDIO)
                    return JamiStrings.audioCodecs
            }
            fontSize: JamiTheme.settingsFontSize
        }

        PushButton {
            source: JamiResources.arrow_drop_down_24dp_svg
            imageColor: JamiTheme.textColor
            onClicked: decreaseCodecPriority()
        }

        PushButton {
            source: JamiResources.arrow_drop_up_24dp_svg
            imageColor: JamiTheme.textColor
            onClicked: increaseCodecPriority()
        }
    }

    JamiListView {
        id: mediaListWidget

        Layout.fillWidth: true
        Layout.preferredHeight: 190

        model: MediaCodecListModel {
            mediaType: root.mediaType
            lrcInstance: LRCInstance
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
                AvAdapter.enableCodec(idToSet, isToBeEnabled)
                updateCodecs()
            }
        }
    }
}
