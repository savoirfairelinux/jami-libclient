/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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

import net.jami.Models 1.1
import net.jami.Adapters 1.1

ProgressBar {
    id: root

    property real rmsLevel: 0

    value: {
        return clamp(rmsLevel * 300.0, 0.0, 100.0)
    }

    onVisibleChanged: {
        if (visible) {
            rmsLevel = 0
            AvAdapter.startAudioMeter()
        } else
            AvAdapter.stopAudioMeter()
    }

    function clamp(num, a, b) {
        return Math.max(Math.min(num, Math.max(a, b)), Math.min(a, b))
    }

    Connections{
        target: AVModel
        enabled: root.visible

        function onAudioMeter(id, level) {
            if (id === "audiolayer_id") {
                rmsLevel = level
            }
        }
    }
}
