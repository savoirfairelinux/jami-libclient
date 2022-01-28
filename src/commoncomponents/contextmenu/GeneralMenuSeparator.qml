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

import net.jami.Constants 1.0

MenuSeparator {
    id: menuSeparator

    property int separatorPreferredWidth: JamiTheme.menuItemsPreferredWidth
    property int separatorPreferredHeight: 1
    property string separatorColor: JamiTheme.tabbarBorderColor

    padding: 0
    topPadding: 1
    bottomPadding: 1
    contentItem: Rectangle {
        implicitWidth: separatorPreferredWidth
        implicitHeight: separatorPreferredHeight
        color: separatorColor
    }
}
