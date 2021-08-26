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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ItemDelegate {
    id: root

    property string deviceName : ""
    property string deviceId : ""
    property bool isCurrent : false

    property bool editable : false

    signal btnRemoveDeviceClicked

    highlighted: ListView.isCurrentItem

    background: Rectangle {
        color: highlighted? JamiTheme.selectedColor : JamiTheme.editBackgroundColor
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
            id: deviceInfoColumnLayout

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            MaterialLineEdit {
                id: editDeviceName

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                padding: 8
                font.pointSize: JamiTheme.textFontSize

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                wrapMode: Text.NoWrap
                readOnly: !editable
                loseFocusWhenEnterPressed: true
                backgroundColor: JamiTheme.editBackgroundColor

                onEditingFinished: {
                    AvAdapter.setDeviceName(editDeviceName.text)
                    editable = !editable
                }
                onReadOnlyChanged: {
                    if (readOnly)
                        editDeviceName.text = Qt.binding(function() {
                            return elidedTextDeviceName.elidedText
                        })
                    else
                        editDeviceName.text = deviceName
                }

                TextMetrics {
                    id: elidedTextDeviceName

                    font: editDeviceName.font
                    elide: Text.ElideRight
                    elideWidth: editDeviceName.width - editDeviceName.leftPadding * 2
                    text: deviceName
                }
            }

            Text {
                id: labelDeviceId

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.leftMargin: editDeviceName.leftPadding

                elide: Text.ElideRight
                color: JamiTheme.textColor
                text: deviceId === "" ? qsTr("Device Id") : deviceId
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
                    if (!editable)
                        editable = !editable
                    else
                        editDeviceName.focus = false
                } else {
                    btnRemoveDeviceClicked()
                }
            }
        }
    }

    CustomBorder {
        commonBorder: false
        lBorderwidth: 0
        rBorderwidth: 0
        tBorderwidth: 0
        bBorderwidth: 2
        borderColor: JamiTheme.selectedColor
    }
}
