/*
 * Copyright (C) 2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Controls 2.12

// UI dev tool to visualize components/layouts
Rectangle {
    property alias name: label.text
    property bool stretchParent: false

    border.width: 1
    color: {
        var r = Math.random() * 0.5 + 0.5;
        var g = Math.random() * 0.5 + 0.5;
        var b = Math.random() * 0.5 + 0.5;
        Qt.rgba(r, g, b, 0.5);
    }
    anchors.fill: parent
    focus: false
    Component.onCompleted: {
        // fallback to some description of the object
        if (label.text === "")
            label.text = this.toString();

        // force the parent to be at least the dimensions of children
        if (stretchParent) {
            parent.width = Math.max(parent.width, parent.childrenRect.width);
            parent.height = Math.max(parent.height, parent.childrenRect.height);
        }
    }

    Label {
        id: label

        anchors.centerIn: parent
    }

}
