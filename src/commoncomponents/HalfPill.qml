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

import QtQuick

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
                                  type === HalfPill.Right
        property bool direction: type === HalfPill.Right ||
                                 type === HalfPill.Bottom

        property bool bp: type === HalfPill.None

        radius: root.radius
        width: root.size + radius * !bp
        height: root.size + radius * !bp
        anchors.fill: root
        anchors.leftMargin: horizontal * direction * -radius * !bp
        anchors.rightMargin: horizontal * !direction * -radius * !bp
        anchors.topMargin: !horizontal * direction * -radius * !bp
        anchors.bottomMargin: !horizontal * !direction * -radius * !bp
    }
}
