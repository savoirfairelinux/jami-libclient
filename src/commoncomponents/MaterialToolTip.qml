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
import QtQuick.Controls

import net.jami.Constants 1.1

ToolTip {
    id: root

    onVisibleChanged: {
        if (visible)
            animation.start()
    }

    contentItem: Text {
        id: label
        text: root.text
        font: root.font
        color: "white"
    }

    background: Rectangle {
        color: "#c4272727"
        radius: 5
    }

    ParallelAnimation {
        id: animation
        NumberAnimation {
             target: background; properties: "opacity"
             from: 0; to: 1.0
             duration: JamiTheme.shortFadeDuration
        }
        NumberAnimation {
             target: background; properties: "scale"
             from: 0.5; to: 1.0
             duration: JamiTheme.shortFadeDuration * 0.5
        }
    }
}
