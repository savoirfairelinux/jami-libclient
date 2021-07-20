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

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ItemDelegate {
    id: root

    property string deviceName : ""
    property string deviceId : ""
    property bool isCurrent : false

    property bool editable : false

    signal btnRemoveDeviceClicked

    function toggleEditable() {
        editable = !editable
        if (!editable) {
           SettingsAdapter.setDeviceName(elidedTextDeviceName.text)
        }
    }

    background: Rectangle {
        color: highlighted? JamiTheme.selectedColor : JamiTheme.editBackgroundColor
    }
    highlighted: ListView.isCurrentItem

    CustomBorder {
        commonBorder: false
        lBorderwidth: 0
        rBorderwidth: 0
        tBorderwidth: 0
        bBorderwidth: 2
        borderColor: JamiTheme.selectedColor
    }

    RowLayout {
        anchors.fill: root

        Image {
            id: deviceImage

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            Layout.leftMargin: JamiTheme.preferredMarginSize

            layer {
                enabled: true
                effect: ColorOverlay {
                    color: JamiTheme.textColor
                }
            }
            source: JamiResources.baseline_desktop_windows_24dp_svg
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            MaterialLineEdit {
                id: editDeviceName

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                font.pointSize: JamiTheme.textFontSize

                wrapMode: Text.NoWrap
                readOnly: !editable
                backgroundColor: JamiTheme.editBackgroundColor
                text: elidedTextDeviceName.elidedText
                padding: 8
            }

            TextMetrics {
                id: elidedTextDeviceName

                elide: Text.ElideRight
                elideWidth: root.width - btnEditDevice.width - deviceImage.width
                            - editDeviceName.leftPadding
                text: deviceName
            }

            ElidedTextLabel {
                id: labelDeviceId

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.leftMargin: editDeviceName.leftPadding

                maxWidth: root.width - btnEditDevice.width - deviceImage.width
                eText: deviceId === "" ? qsTr("Device Id") : deviceId
            }
        }

        PushButton {
            id: btnEditDevice

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 16
            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            imageColor: JamiTheme.textColor
            normalColor: highlighted ?
                             JamiTheme.selectedColor :
                             JamiTheme.editBackgroundColor

            source: isCurrent ?
                        (editable ?
                             JamiResources.round_save_alt_24dp_svg :
                             JamiResources.round_edit_24dp_svg) :
                        JamiResources.round_remove_circle_24dp_svg

            toolTipText: isCurrent ?
                             (editable ?
                                  JamiStrings.saveNewDeviceName :
                                  JamiStrings.editDeviceName) :
                             JamiStrings.unlinkDevice

            onClicked: {
                if (isCurrent) {
                    toggleEditable()
                } else {
                    btnRemoveDeviceClicked()
                }
            }
        }
    }
}
