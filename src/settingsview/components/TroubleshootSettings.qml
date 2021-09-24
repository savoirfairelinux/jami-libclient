/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Trevor Tabah <trevor.tabah@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"
import "../js/logviewwindowcreation.js" as LogViewWindowCreation

ColumnLayout {
    id: root

    property int itemWidth

    Label {
        Layout.fillWidth: true

        text: JamiStrings.troubleshootTitle
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
        color: JamiTheme.textColor

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    RowLayout {
        Layout.leftMargin: JamiTheme.preferredMarginSize

        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            Layout.rightMargin: JamiTheme.preferredMarginSize

            text: JamiStrings.troubleshootText
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter

            color: JamiTheme.textColor
        }

        MaterialButton {
            id: enableTroubleshootingButton

            Layout.alignment: Qt.AlignRight

            preferredWidth: itemWidth / 1.5
            preferredHeight: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            text: JamiStrings.troubleshootButton

            onClicked: {
                LogViewWindowCreation.createlogViewWindowObject()
                LogViewWindowCreation.showLogViewWindow()
            }
        }
    }
}
