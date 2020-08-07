
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
import QtQuick 2.15
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import net.jami.Models 1.0


/*
 * HoverableButton contains the following configurable properties:
 * 1. Color changes on different button state
 * 2. Radius control (rounded)
 * 3. Text content or image content
 * 4. Can use OnClicked slot to implement some click logic
 */
Button {
    id: hoverableButton
    property int fontPointSize: 9
    property string backgroundColor: JamiTheme.releaseColor
    property string backgroundColorDisabled : Qt.rgba(242/256, 242/256, 242/256, 0.8)

    property string startColor :"#109ede"
    property string endColor : "#2b5084"

    property string startColorPressed :"#043161"
    property string endColorPressed : "#00113f"

    property string startColorHovered :"#2b4b7e"
    property string endColorHovered : "#001d4d"

    property string onPressColor: JamiTheme.pressColor
    property string onReleaseColor: backgroundColor
    property string onEnterColor: JamiTheme.hoverColor
    property string onExitColor: backgroundColor
    property string textColor: "white"

    property string toolTipText: ""

    property alias radius: hoverableButtonBackground.radius

    property bool isBeingPressed: false

    radius: height / 2
    font.pointSize: fontPointSize
    font.kerning:  true
    hoverEnabled: true

    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.visible: hovered && (toolTipText.length > 0)
    ToolTip.text: toolTipText

    contentItem: Text {
            text: hoverableButton.text
            font: hoverableButton.font
            opacity: enabled ? 1.0 : 0.3
            color: enabled? textColor : "grey"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

    background: Rectangle {
        id: hoverableButtonBackground
        color: backgroundColor

        MouseArea {
            id: btnMouseArea
            anchors.fill: hoverableButtonBackground
            hoverEnabled: true
            onPressed: {
                hoverableButtonBackground.color = onPressColor
                isBeingPressed = true
            }
            onReleased: {
                hoverableButtonBackground.color = onReleaseColor
                isBeingPressed = false
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

    LinearGradient {
        id: backgroundGradient

        source: hoverableButtonBackground
        anchors.fill: hoverableButtonBackground
        start: Qt.point(0, 0)
        end: Qt.point(width, 0)
        gradient: Gradient {
            GradientStop { position: 0.0; color: {
                    if(!hoverableButton.enabled){
                        return backgroundColorDisabled
                    }

                if(isBeingPressed){
                    return startColorPressed
                } else {
                    if(hoverableButton.hovered){
                        return startColorHovered
                    } else {
                        return startColor
                    }
                }
                } }

            GradientStop { position: 1.0; color: {
                    if(!hoverableButton.enabled){
                        return backgroundColorDisabled
                    }

                    if(isBeingPressed){
                        return endColorPressed
                    } else {
                        if(hoverableButton.hovered){
                            return endColorHovered
                        } else {
                            return endColor
                        }
                    }
                    } }
        }
    }
}
