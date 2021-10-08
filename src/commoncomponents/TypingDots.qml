/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

import net.jami.Constants 1.1

Row {
    id: root

    property int currentRect: 0

    spacing: 5

    Timer {
        repeat: true
        running: true
        interval: JamiTheme.typingDotsAnimationInterval

        onTriggered: {
            if (root.currentRect < 2)
                root.currentRect ++
            else
                root.currentRect = 0
        }
    }

    Repeater {
        model: 3

        Rectangle {
            id: circleRect

            radius: JamiTheme.typingDotsRadius

            width: JamiTheme.typingDotsSize
            height: JamiTheme.typingDotsSize
            color: JamiTheme.typingDotsNormalColor

            states: State {
                id: enlargeState

                name: "enlarge"
                when: root.currentRect === index
            }

            transitions: [
                Transition {
                    to: "enlarge"
                    ParallelAnimation {
                        NumberAnimation {
                            from: 1.0
                            to: 1.3
                            target: circleRect
                            duration: JamiTheme.typingDotsAnimationInterval
                            property: "scale"
                        }

                        ColorAnimation {
                            from: JamiTheme.typingDotsNormalColor
                            to: JamiTheme.typingDotsEnlargeColor
                            target: circleRect
                            property: "color"
                            duration: JamiTheme.typingDotsAnimationInterval
                        }
                    }
                },
                Transition {
                    from: "enlarge"
                    ParallelAnimation {
                        NumberAnimation {
                            from: 1.3
                            to: 1.0
                            target: circleRect
                            duration: JamiTheme.typingDotsAnimationInterval
                            property: "scale"
                        }
                        ColorAnimation {
                            from: JamiTheme.typingDotsEnlargeColor
                            to: JamiTheme.typingDotsNormalColor
                            target: circleRect
                            property: "color"
                            duration: JamiTheme.typingDotsAnimationInterval
                        }
                    }
                }
            ]
        }
    }
}
