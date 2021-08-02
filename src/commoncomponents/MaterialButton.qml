/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien blin <sebastien.blin@savoirfairelinux.com>
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
import QtQuick.Window 2.14

import net.jami.Constants 1.0

Button {
    id: root

    property alias fontCapitalization: buttonText.font.capitalization
    property alias source: buttonImage.source
    property alias toolTipText: toolTip.text
    property var color: "transparent"
    property var hoveredColor: undefined
    property var pressedColor: undefined
    property var keysNavigationFocusColor: Qt.darker(hoveredColor, 2)
    property var outlined: false
    property string animatedImageSource: ""

    property var preferredWidth: 400
    property var preferredHeight: 36
    property var minimumIconTextSpacing: 10
    property var iconPreferredHeight: 18
    property var iconPreferredWidth: 18

    property bool enableElide: true
    property int elide: Text.ElideRight
    property int textActualWidth: buttonTextMetrics.boundingRect.width

    font.kerning: true
    font.pointSize: JamiTheme.textFontSize

    hoverEnabled: hoveredColor !== undefined

    MaterialToolTip {
        id: toolTip

        parent: root
        visible: hovered && (toolTipText.length > 0)
        delay: Qt.styleHints.mousePressAndHoldInterval
    }

    contentItem: Item {
        Rectangle {
            anchors.fill: parent
            color: "transparent"

            AnimatedImage {
                id: buttonAnimatedImage

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: JamiTheme.preferredMarginSize / 2

                height: iconPreferredHeight
                width: iconPreferredWidth

                source: animatedImageSource
                playing: true
                paused: false
                fillMode: Image.PreserveAspectFit
                mipmap: true
                visible: animatedImageSource.length !== 0
            }

            ResponsiveImage {
                id: buttonImage

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: JamiTheme.preferredMarginSize / 2

                height: iconPreferredHeight
                width: iconPreferredWidth

                visible: source.toString().length !== 0
                color: {
                    if (!outlined)
                        return "white"
                    if (hovered && root.hoveredColor)
                        return root.hoveredColor
                    if (checked && root.pressedColor)
                        return root.pressedColor
                    return root.color
                }
            }

            Text {
                id: buttonText

                anchors.centerIn: parent

                width: {
                    var iconWidth = (buttonAnimatedImage.visible || buttonImage.visible) ?
                                iconPreferredWidth : 0
                    return (parent.width / 2 - iconWidth -
                            JamiTheme.preferredMarginSize / 2 - minimumIconTextSpacing) * 2
                }

                TextMetrics {
                    id: buttonTextMetrics

                    font: buttonText.font
                    text: buttonText.text
                }

                text: root.text
                elide: enableElide ? root.elide : Text.ElideNone
                color: {
                    if (!outlined)
                        return "white"
                    if (hovered && root.hoveredColor)
                        return root.hoveredColor
                    if (checked && root.pressedColor)
                        return root.pressedColor
                    return root.color
                }
                font: root.font
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    background: Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: {
            if (outlined)
                return "transparent"
            if (hovered && root.hoveredColor)
                return root.hoveredColor
            if (checked && root.pressedColor)
                return root.pressedColor
            return root.focus ? root.keysNavigationFocusColor : root.color
        }
        border.color: {
            if (!outlined)
                return "transparent"
            if (hovered && root.hoveredColor)
                return root.hoveredColor
            if (checked && root.pressedColor)
                return root.pressedColor
            return root.focus ? root.keysNavigationFocusColor : root.color
        }
        radius: 4
    }

    Keys.onPressed: function (keyEvent) {
        if (keyEvent.matches(StandardKey.InsertParagraphSeparator)) {
            clicked()
            keyEvent.accepted = true
        }
    }
}
