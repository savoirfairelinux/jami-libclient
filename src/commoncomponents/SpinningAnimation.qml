/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.12

Item {
    id: root

    ConicalGradient {
        anchors.fill: parent
        angle: 0.0
        gradient: Gradient {
            GradientStop { position: 0.5; color: "transparent" }
            GradientStop { position: 1.0; color: "white" }
        }

        RotationAnimation on angle {
            loops: Animation.Infinite
            duration: 1000
            from: 0
            to: 360
        }
    }
    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Rectangle {
            width: root.height
            height: root.height
            radius: root.height / 2
        }
    }
}