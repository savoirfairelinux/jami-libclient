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

import QtQuick
import QtQuick.Controls

import net.jami.Adapters 1.1

import "../../commoncomponents"

Item {
    id: root

    property alias imageId: avatar.imageId
    property alias showPresenceIndicator: avatar.showPresenceIndicator
    property alias animationMode: animation.mode

    SpinningAnimation {
        id: animation

        anchors.fill: root
    }

    Avatar {
        id: avatar

        anchors.fill: root
        anchors.margins: animation.mode === SpinningAnimation.Mode.Disabled ?
                             0 :
                             animation.spinningAnimationWidth

        mode: Avatar.Mode.Conversation
    }
}
