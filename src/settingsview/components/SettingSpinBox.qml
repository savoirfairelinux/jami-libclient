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

RowLayout {
    id: root

    property alias title: title.text
    property alias enabled: textField.enabled
    property alias bottomValue: textFieldValidator.bottom
    property alias topValue: textFieldValidator.top
    property alias valueField: textField.text
    property alias tooltipText: toolTip.text
    property alias inputAcceptable: textField.acceptableInput

    property string borderColor: JamiTheme.greyBorderColor
    property int itemWidth

    signal newValue

    Text {
        id: title

        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        color: JamiTheme.textColor
        elide: Text.ElideRight
        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true
        verticalAlignment: Text.AlignVCenter
    }

    TextField {
        id: textField

        Layout.preferredWidth: root.itemWidth
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.alignment: Qt.AlignCenter

        font.pointSize: JamiTheme.buttonFontSize
        font.kerning: true

        validator: IntValidator {
            id: textFieldValidator
        }

        color: JamiTheme.textColor
        hoverEnabled: true

        background: Rectangle {
            border.color: enabled ? root.borderColor : JamiTheme.transparentColor
            color: JamiTheme.editBackgroundColor
            radius: JamiTheme.primaryRadius
        }

        onEditingFinished: newValue()

        Keys.onPressed: {
            if (event.key === Qt.Key_Enter ||
                    event.key === Qt.Key_Return) {
                textField.focus = false
                event.accepted = true
            }
        }

        MaterialToolTip {
            id: toolTip

            parent: textField
            visible: textField.hovered && (root.tooltipText.length > 0)
            delay: Qt.styleHints.mousePressAndHoldInterval
        }
    }
}
