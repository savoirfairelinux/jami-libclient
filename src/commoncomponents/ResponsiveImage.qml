/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Tracyk <andreas.traczyk@savoirfairelinux.com>
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

import net.jami.Models 1.0

Image {
    id: root

    property real containerWidth
    property real containerHeight

    property int padding: 0
    property point offset: Qt.point(0, 0)

    property string normalSource
    property string checkedSource
    property string color: "transparent"

    property bool isSvg: {
        var match = /[^.]+$/.exec(source)
        return match !== null && match[0] === 'svg'
    }

    anchors.horizontalCenterOffset: offset.x
    anchors.verticalCenterOffset: offset.y

    // works out to 24 if containerWidth is 30
    width: Math.trunc(containerWidth * Math.sqrt(2) * 0.5) + 3 - padding
    height: Math.trunc(containerHeight * Math.sqrt(2) * 0.5) + 3 - padding

    fillMode: Image.PreserveAspectFit
    smooth: false
    antialiasing: false
    asynchronous: true

    layer {
        enabled: true
        effect: ColorOverlay {
            id: overlay
            color: root.color
        }
    }

    function setSourceSize() {
        if (isSvg) {
            sourceSize = Qt.size(0, 0)
            sourceSize = Qt.size(width, height)
        }
        else
            sourceSize = undefined
    }

    Connections {
        target: ScreenInfo

        function onDevicePixelRatioChanged(){
            setSourceSize()
        }
    }

    Component.onCompleted: setSourceSize()
}
