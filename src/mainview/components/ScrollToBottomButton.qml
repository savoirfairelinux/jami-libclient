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
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

import net.jami.Constants 1.1

import "../../commoncomponents"

Control {
    id: root

    property alias activeStateTrigger: activeState.when

    signal clicked

    height: jumpToLatestText.contentHeight + 15
    width: jumpToLatestText.contentWidth + arrowDropDown.width + 50

    opacity: 0

    states: State {
        id: activeState

        name: "active"
        PropertyChanges {
            target: root
            opacity: 1
        }
    }

    transitions: [
        Transition {
            to: "active"
            NumberAnimation {
                target: root
                duration: JamiTheme.shortFadeDuration
                property: "opacity"
            }
        },
        Transition {
            from: "active"
            NumberAnimation {
                target: root
                duration: JamiTheme.shortFadeDuration
                property: "opacity"
                to: 0.0
            }
        }
    ]

    contentItem: Item {
        Item {
            anchors.centerIn: parent

            height: jumpToLatestText.contentHeight
            width: jumpToLatestText.contentWidth + arrowDropDown.width + 3

            Text {
                id: jumpToLatestText

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter

                font.weight: Font.DemiBold
                font.pointSize: JamiTheme.textFontSize + 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: JamiStrings.jumpToLatest
                color: JamiTheme.whiteColor
            }

            ResponsiveImage {
                id: arrowDropDown

                anchors.left: jumpToLatestText.right
                anchors.leftMargin: 3
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 2

                containerWidth: 12
                containerHeight: 12

                color: JamiTheme.whiteColor
                source: JamiResources.down_triangle_arrow_black_24dp_svg
            }
        }
    }

    background: Rectangle {
        radius: 20
        color: JamiTheme.jamiDarkBlue

        MouseArea {
            anchors.fill: parent
            cursorShape: root.opacity ? Qt.PointingHandCursor :
                                        Qt.ArrowCursor

            onClicked: root.clicked()
        }

        layer {
            enabled: true
            effect: DropShadow {
                horizontalOffset: 3.0
                verticalOffset: 3.0
                radius: 8.0
                samples: 16
                color: JamiTheme.shadowColor
            }
        }
    }
}
