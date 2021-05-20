/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

Item {
    id: root

    enum Type {
        None,
        Top,
        Left,
        Bottom,
        Right
    }

    property int type: HalfPill.None
    property int radius: 0
    property alias color: rect.color

    clip: true

    Rectangle {
        id: rect

        property bool horizontal: type === HalfPill.Left ||
                                  type == HalfPill.Right
        property bool direction: type === HalfPill.Right ||
                                 type == HalfPill.Bottom

        radius: root.radius * (type !== HalfPill.None)
        width: root.size + radius
        height: root.size + radius
        anchors.fill: root
        anchors.leftMargin: horizontal * direction * -radius
        anchors.rightMargin: horizontal * !direction * -radius
        anchors.topMargin: !horizontal * direction * -radius
        anchors.bottomMargin: !horizontal * !direction * -radius
    }
}
