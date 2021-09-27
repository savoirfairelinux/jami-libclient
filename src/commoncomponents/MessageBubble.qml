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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtGraphicalEffects 1.0

import net.jami.Constants 1.1

Rectangle {
    id: root

    property bool out: true
    property int type: MsgSeq.single

    Rectangle {
        id: mask

        visible: type !== MsgSeq.single
        z: -1
        radius: 2
        color: root.color

        anchors {
            fill: parent
            leftMargin: out ? root.width - root.radius : 0
            rightMargin: out ? 0 : root.width - root.radius
            topMargin: type === MsgSeq.first ? root.height - root.radius : 0
            bottomMargin: type === MsgSeq.last ? root.height - root.radius : 0
        }
    }
}
