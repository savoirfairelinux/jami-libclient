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

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

RowLayout {
    id: root

    property string borderColor: JamiTheme.greyBorderColor
    property string title: ""
    property int itemWidth
    property int bottomValue
    property int topValue
    property int step
    property int valueField
    property string tooltipText: ""

    signal newValue

    function setEnabled(status) {
        textField.enabled = status
    }

    function setValue(value) {
        root.valueField = value
        textField.text = value
    }

    Text {
        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        text: root.title
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

        font.family: "Monospace"
        font.pointSize: JamiTheme.buttonFontSize
        font.kerning: true

        validator: IntValidator {bottom: root.bottomValue; top: root.topValue}

        onEditingFinished: {
            root.valueField = text
            newValue()
        }

        color: JamiTheme.textColor

        background: Rectangle {
            border.color: enabled? root.borderColor : "transparent"
            color: JamiTheme.editBackgroundColor
        }

        hoverEnabled: true
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
        ToolTip.visible: hovered && (root.tooltipText.length > 0)
        ToolTip.text: root.tooltipText
    }
}
