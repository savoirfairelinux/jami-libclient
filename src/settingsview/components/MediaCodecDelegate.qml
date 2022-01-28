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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0

ItemDelegate {
    id: root

    property string mediaCodecName : ""
    property bool isEnabled : false
    property int mediaCodecId
    property string samplerRate: ""
    property int checkBoxWidth: 24
    property int mediaType

    signal mediaCodecStateChange(string idToSet , bool isToBeEnabled)

    highlighted: ListView.isCurrentItem

    RowLayout {
        anchors.fill: parent

        CheckBox {
            id: checkBoxIsEnabled

            Layout.leftMargin: 20
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillHeight: true
            Layout.preferredWidth: checkBoxWidth

            tristate: false
            checkState: isEnabled ? Qt.Checked : Qt.Unchecked

            text: ""
            indicator: Image {
                anchors.centerIn: parent
                width: checkBoxWidth
                height: checkBoxWidth
                source: checkBoxIsEnabled.checked ?
                    "qrc:/images/icons/check_box-24px.svg" :
                    "qrc:/images/icons/check_box_outline_blank-24px.svg"
            }

            nextCheckState: function() {
                    var result
                    var result_bool

                    if (checkState === Qt.Checked) {
                        result = Qt.Unchecked
                        result_bool = false
                    } else {
                        result = Qt.Checked
                        result_bool = true
                    }
                    mediaCodecStateChange(mediaCodecId, result_bool)
                    return result
                }
        }

        Label {
            id: formatNameLabel

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: JamiTheme.preferredMarginSize / 2

            text: {
                if (mediaType == MediaSettings.VIDEO)
                    return mediaCodecName
                else if (mediaType == MediaSettings.AUDIO)
                    return mediaCodecName + " " + samplerRate + " Hz"
            }
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 8
            font.kerning: true
        }
    }
}
