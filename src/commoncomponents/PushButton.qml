/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

import net.jami.Constants 1.0

//
// PushButton contains the following configurable properties:
// - colored states
// - radius
// - minimal support for text
// - animation duration
// TODO: allow transparent background tinted text/icon
//
AbstractButton {
    id: root

    // Shape will default to a 15px circle
    // but can be sized accordingly.
    property int preferredSize: 30
    property int preferredWidth: preferredSize
    property int preferredHeight: preferredSize
    property int preferredMargin: 16
    property alias textHAlign: textContent.horizontalAlignment
    // Note the radius will default to preferredSize
    property alias radius: background.radius

    // Text properties
    property alias buttonText: textContent.text
    property alias buttonTextColor: textContent.color
    property alias fontPointSize: textContent.font.pointSize

    property string toolTipText: ""

    // State colors
    property string pressedColor: JamiTheme.pressedButtonColor
    property string hoveredColor: JamiTheme.hoveredButtonColor
    property string normalColor: JamiTheme.normalButtonColor
    property string checkedColor: pressedColor

    // State transition duration
    property int duration: JamiTheme.shortFadeDuration

    // Image properties
    property alias source: image.source
    property var imageColor: null
    property string normalImageSource
    property var checkedImageColor: null
    property string checkedImageSource
    property alias imagePadding: image.padding
    property alias imageOffset: image.offset

    width: preferredSize
    height: preferredSize

    checkable: false
    checked: false
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.visible: hovered && (toolTipText.length > 0)
    ToolTip.text: toolTipText

    background: Rectangle {
        id: background

        radius: preferredSize

        states: [
            State {
                name: "checked"; when: checked
                PropertyChanges { target: background; color: checkedColor }
            },
            State {
                name: "pressed"; when: pressed
                PropertyChanges { target: background; color: pressedColor }
            },
            State {
                name: "hovered"; when: hovered
                PropertyChanges { target: background; color: hoveredColor }
            },
            State {
                name: "normal"; when: !hovered && ! checked
                PropertyChanges { target: background; color: normalColor }
            }
        ]

        transitions: [
            Transition {
                to: "normal"; reversible: true; enabled: duration
                ColorAnimation { duration: root.duration }
            },
            Transition {
                to: "pressed"; reversible: true; enabled: duration
                ColorAnimation { duration: root.duration * 0.5 }
            },
            Transition {
                to: ""; reversible: true; enabled: duration
                ColorAnimation { duration: root.duration }
            }
        ]

        ResponsiveImage {
            id: image

            containerWidth: preferredSize
            containerHeight: preferredSize

            anchors.verticalCenter: background.verticalCenter
            anchors.horizontalCenter: textContent.text ? undefined : parent.horizontalCenter
            anchors.left: textContent.text ? parent.left : undefined
            anchors.leftMargin: preferredMargin

            source: {
                if (checkable && checkedImageSource)
                    return checked ? checkedImageSource : normalImageSource
                else
                    return normalImageSource
            }

            layer {
                enabled: imageColor || checkedColor
                effect: ColorOverlay {
                    id: overlay
                    color: {
                        if (checked && checkedImageColor)
                            return checkedImageColor
                        else if (imageColor)
                            return imageColor
                        else
                            return JamiTheme.transparentColor
                    }
                }
                // Mipmap does not render correctly on linux
                mipmap: false
                smooth: true
            }
        }

        Text {
            id: textContent

            anchors.left: {
                if (image.source.toString() !== '')
                    return image.right
                else
                    return background.left
            }
            anchors.leftMargin: preferredMargin
            anchors.right: background.right
            anchors.rightMargin: preferredMargin
            anchors.verticalCenter: background.verticalCenter

            horizontalAlignment: Text.AlignHCenter

            color: JamiTheme.primaryForegroundColor
            font.kerning:  true
            font.pointSize: 9
            elide: Qt.ElideRight
        }
    }
}
