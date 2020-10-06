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
import QtQuick.Window 2.15

Image {
    id: root

    property real containerWidth
    property real containerHeight

    property int padding: 0
    property point offset: Qt.point(0, 0)

    property string normalSource
    property string checkedSource

    property real pixelDensity: Screen.pixelDensity
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

    function setSourceSize() {
        if (isSvg) {
            sourceSize.width = Math.max(24, width)
            sourceSize.height = Math.max(24, height)
        } else
            sourceSize = undefined
    }

    onPixelDensityChanged: setSourceSize()
    Component.onCompleted: setSourceSize()
}
