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

import QtQuick
import QtQuick.Controls

import net.jami.Constants 1.1

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
    property int preferredHeight: 0
    property int preferredWidth: 0
    property int preferredMargin: 16
    // Note the radius will default to preferredSize
    property alias radius: background.radius

    // Text properties
    property alias buttonText: textContent.text
    property alias buttonTextHeight: textContent.height
    readonly property alias buttonTextWidth: textContent.width
    property alias buttonTextFont: textContent.font
    property alias buttonTextColor: textContent.color
    property alias textHAlign: textContent.horizontalAlignment
    property bool buttonTextEnableElide: false

    property alias toolTipText: toolTip.text

    // State colors
    property string pressedColor: JamiTheme.pressedButtonColor
    property string hoveredColor: JamiTheme.hoveredButtonColor
    property string normalColor: JamiTheme.normalButtonColor
    property string checkedColor: pressedColor

    // State transition duration
    property int duration: JamiTheme.shortFadeDuration

    // Image properties
    property alias imageContainerWidth: image.containerWidth
    property alias imageContainerHeight: image.containerHeight
    property alias source: image.source
    property var imageColor: null
    property string normalImageSource
    property var checkedImageColor: null
    property string checkedImageSource
    property alias imagePadding: image.padding
    property alias imageOffset: image.offset

    width: preferredWidth ? preferredWidth : preferredSize
    height: preferredHeight ? preferredHeight : preferredSize

    checkable: false
    checked: false
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.role: Accessible.Button
    Accessible.name: buttonText
    Accessible.description: toolTipText

    MaterialToolTip {
        id: toolTip

        parent: root
        visible: hovered && (toolTipText.length > 0)
        delay: Qt.styleHints.mousePressAndHoldInterval
    }

    ResponsiveImage {
        id: image

        anchors.centerIn: textContent.text ? undefined : root
        anchors.left: textContent.text ? root.left : undefined
        anchors.leftMargin: textContent.text ? preferredMargin : 0
        anchors.verticalCenter: root.verticalCenter

        containerWidth: preferredWidth ? preferredWidth : preferredSize
        containerHeight: preferredHeight ? preferredHeight : preferredSize

        source: {
            if (checkable && checkedImageSource)
                return checked ? checkedImageSource : normalImageSource
            else
                return normalImageSource
        }

        color: {
            if (checked && checkedImageColor)
                return checkedImageColor
            else if (imageColor)
                return imageColor
            else
                return JamiTheme.transparentColor
        }
    }

    Text {
        id: textContent

        anchors.centerIn: image.status !== Image.Null ? undefined : root
        anchors.left: image.status !== Image.Null ? image.right : undefined
        anchors.leftMargin: preferredMargin
        anchors.verticalCenter: root.verticalCenter

        anchors.right: buttonTextEnableElide ? root.right : undefined
        anchors.rightMargin: preferredMargin

        visible: text ? true : false

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        color: JamiTheme.primaryForegroundColor
        font.kerning: true
        font.pointSize: 9
        elide: Qt.ElideRight
    }

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
                name: "hovered"; when: hovered || root.focus
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
    }
}
