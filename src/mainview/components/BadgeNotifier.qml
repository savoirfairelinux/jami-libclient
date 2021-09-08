/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

import QtQuick

import net.jami.Constants 1.1

Rectangle {
    id: root

    property real size
    property int count: 0
    property int lastCount: count
    property bool populated: false
    property bool animate: true

    width: size
    height: size

    radius: JamiTheme.primaryRadius
    color: JamiTheme.filterBadgeColor

    visible: count > 0

    Text {
        id: countLabel

        anchors.centerIn: root
        text: count > 9 ? "…" : count
        color: JamiTheme.filterBadgeTextColor
        font.pointSize: JamiTheme.filterBadgeFontSize
        font.weight: Font.ExtraBold
    }

    onCountChanged: {
        if (count > lastCount && animate)
            notifyAnim.start()
        lastCount = count
        if (!populated)
            populated = true
    }
    ParallelAnimation {
        id: notifyAnim

        ColorAnimation {
            target: root; properties: "color"
            from: JamiTheme.filterBadgeTextColor
            to: JamiTheme.filterBadgeColor
            duration: 150; easing.type: Easing.InOutQuad
        }
        ColorAnimation {
            target: countLabel; properties: "color"
            from: JamiTheme.filterBadgeColor
            to: JamiTheme.filterBadgeTextColor
            duration: 150; easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: root; property: "y"
            from: -3; to: 0
            duration: 150; easing.type: Easing.InOutQuad
        }
    }
}
