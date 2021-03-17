/*
 * Copyright (C) 2020 by Savoir-faire Linux
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
import QtGraphicalEffects 1.14

import net.jami.Constants 1.0

Menu {
    id: root

    property int menuItemsPreferredWidth: 220
    property int menuItemsPreferredHeight: 48
    property int generalMenuSeparatorCount: 0
    property int commonBorderWidth: 1
    font.pointSize: JamiTheme.menuFontSize

    function openMenu(){
        visible = true
        visible = false
        visible = true
    }

    background: Rectangle {
        implicitWidth: menuItemsPreferredWidth
        implicitHeight: menuItemsPreferredHeight
                        * (root.count - generalMenuSeparatorCount)

        border.width: commonBorderWidth
        border.color: JamiTheme.tabbarBorderColor
        color: JamiTheme.backgroundColor
    }
}
