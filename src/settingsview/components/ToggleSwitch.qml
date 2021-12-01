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

import net.jami.Constants 1.1

import "../../commoncomponents"

RowLayout {
    id: root
    property string labelText: ""
    property int widthOfSwitch: 50
    property int heightOfSwitch: 10
    property int heightOfLayout: 30
    property int fontPointSize: JamiTheme.headerFontSize

    property string tooltipText: ""

    property alias toggleSwitch: switchOfLayout
    property alias checked: switchOfLayout.checked

    signal switchToggled

    Text {
        Layout.fillWidth: true
        Layout.preferredHeight: heightOfLayout
        Layout.rightMargin: JamiTheme.preferredMarginSize

        text: qsTr(labelText)
        font.pointSize: fontPointSize
        font.kerning: true
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        color: JamiTheme.textColor
    }

    Switch {
        id: switchOfLayout
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        Layout.preferredWidth: widthOfSwitch
        Layout.preferredHeight: heightOfSwitch

        hoverEnabled: true
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
        ToolTip.visible: hovered && (tooltipText.length > 0)
        ToolTip.text: tooltipText

        Accessible.role: Accessible.Button
        Accessible.name: root.labelText
        Accessible.description: root.tooltipText

        onToggled: {
            switchToggled()
        }
    }
}
