
/*
 * Copyright (C) 2020 by Savoir-faire Linux
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
import QtQuick 2.14
import QtQuick.Controls 2.14
import net.jami.Models 1.0
import QtGraphicalEffects 1.15


// HoverableButton contains the following configurable properties:
// 1. Color changes on different button state
// 2. Radius control (rounded)
// 3. Text content or image content
// 4. Can use OnClicked slot to implement some click logic
//
// Note: if use text property directly, buttonTextColor will not work.
Button {
    id: hoverableButton

    checkable: true
    checked: false

    property int fontPointSize: 9
    property int buttonImageHeight: hoverableButtonBackground.height - 10
    property int buttonImageWidth: hoverableButtonBackground.width - 10

    property string buttonText: ""
    property string buttonTextColor: "black"

    property string backgroundColor: JamiTheme.releaseColor
    property string onPressColor: JamiTheme.pressColor
    property string onReleaseColor: JamiTheme.releaseColor
    property string onEnterColor: JamiTheme.hoverColor
    property string onExitColor: JamiTheme.releaseColor

    property alias radius: hoverableButtonBackground.radius
    property alias source: hoverableButtonImage.source
    property var checkedImage: ""
    property var baseImage: ""
    property var checkedColor: null
    property var baseColor: null
    property alias color: hoverableButton.baseColor
    property string toolTipText: ""

    font.pointSize: fontPointSize

    hoverEnabled: true

    text: "<font color=" + "'" + buttonTextColor + "'>" + buttonText + "</font>"

    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.visible: hovered && (toolTipText.length > 0)
    ToolTip.text: toolTipText

    background: Rectangle {
        id: hoverableButtonBackground

        color: backgroundColor

        Image {
            id: hoverableButtonImage

            anchors.centerIn: hoverableButtonBackground

            height: buttonImageHeight
            width: buttonImageWidth

            fillMode: Image.PreserveAspectFit
            mipmap: true
            asynchronous: true

            source: {
                if (checkable && checkedImage)
                    return hoverableButton.checked ? checkedImage : baseImage
                else
                    return ""
            }

            layer {
                enabled: true
                effect: ColorOverlay {
                    id: overlay
                    color: hoverableButton.checked && checkedColor?
                        checkedColor :
                        (baseColor? baseColor : "transparent")
                }
            }
        }

        MouseArea {
            anchors.fill: parent

            hoverEnabled: hoverableButton.hoverEnabled

            onPressed: {
                hoverableButtonBackground.color = onPressColor
            }
            onReleased: {
                hoverableButtonBackground.color = onReleaseColor
                hoverableButton.toggle()
                hoverableButton.clicked()
            }
            onEntered: {
                hoverableButtonBackground.color = onEnterColor
            }
            onExited: {
                hoverableButtonBackground.color = onExitColor
            }
        }
    }
}
