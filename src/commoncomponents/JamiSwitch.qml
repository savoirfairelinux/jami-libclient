/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

import QtQuick 2.14
import QtQuick.Controls 2.14

import net.jami.Constants 1.0

Switch {
    id: root

    indicator: Rectangle {
        implicitWidth: JamiTheme.switchPreferredWidth
        implicitHeight: JamiTheme.switchPreferredHeight

        x: root.leftPadding
        y: parent.height / 2 - height / 2

        radius: JamiTheme.switchIndicatorRadius

        color: root.checked ? JamiTheme.switchBackgroundCheckedColor : JamiTheme.whiteColor
        border.color: root.checked ? JamiTheme.switchBackgroundCheckedColor :
                                     JamiTheme.switchBackgroundBorderColor

        Rectangle {
            x: root.checked ? parent.width - width : 0
            y: parent.height / 2 - height / 2

            width: JamiTheme.switchIndicatorPreferredWidth
            height: JamiTheme.switchIndicatorPreferredHeight

            radius: JamiTheme.switchIndicatorRadius

            color: (root.down || root.focus) ? Qt.darker(JamiTheme.switchBackgroundBorderColor, 1.2) :
                                               JamiTheme.whiteColor
            border.color: root.checked ? JamiTheme.switchBackgroundCheckedColor :
                                         JamiTheme.switchIndicatorBorderColor
        }
    }

    Keys.onPressed: function (keyEvent) {
        if (keyEvent.key === Qt.Key_Enter ||
                keyEvent.key === Qt.Key_Return) {
            checked = !checked
            keyEvent.accepted = true
        }
    }
}
