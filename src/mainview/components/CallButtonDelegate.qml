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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.14
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ItemDelegate {
    id: wrapper

    property bool isFirst: index < 1
    property bool isLast: index + 1 < ListView.view.count ? false : true
    property bool hasLast: ListView.view.centeredGroup !== undefined
    property bool isVertical: wrapper.ListView.view.orientation === ListView.Vertical

    property alias subMenuVisible: menu.popup.visible

    action: ItemAction
    checkable: ItemAction.checkable

    // hide the action's visual elements like the blurry looking icon
    icon.source: ""
    text: ""

    z: index

    // TODO: remove this when output volume control is implemented
    MouseArea {
        visible: ItemAction.bypassMuteAction !== undefined &&
                 ItemAction.bypassMuteAction && !menu.popup.visible
        anchors.fill: wrapper
        onClicked: menu.popup.open()
    }

    background: HalfPill {
        anchors.fill: parent
        radius: 5
        color: {
            if (supplimentaryBackground.visible)
                return "#c4272727"
            return wrapper.down ?
                        "#c4777777" :
                        wrapper.hovered && !menu.hovered ?
                            "#c4444444" :
                            "#c4272727"
        }
        type: {
            if (isVertical) {
                if (isFirst)
                    return HalfPill.Top
                else if (isLast && hasLast)
                    return HalfPill.Bottom
            } else {
                if (isFirst)
                    return HalfPill.Left
                else if (isLast && hasLast)
                     return HalfPill.Right
            }
            return HalfPill.None
        }

        Behavior on color {
            ColorAnimation { duration: JamiTheme.shortFadeDuration }
        }
    }

    Rectangle {
        id: supplimentaryBackground

        visible: ItemAction.hasBg !== undefined
        color: wrapper.down ?
                   Qt.lighter(JamiTheme.refuseRed, 1.5) :
                   wrapper.hovered && !menu.hovered ?
                       JamiTheme.refuseRed :
                       JamiTheme.refuseRedTransparent
        anchors.fill: parent
        radius: width / 2

        Behavior on color {
            ColorAnimation { duration: JamiTheme.shortFadeDuration }
        }
    }

    ResponsiveImage {
        id: icon

        // TODO: remove this when the icons are size corrected
        property real size: ItemAction.size !== undefined ?
                                ItemAction.size : 30
        containerWidth: size
        containerHeight: size

        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        source: ItemAction.icon.source
        color: ItemAction.icon.color

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: ItemAction.blinksWhenChecked !== undefined &&
                     ItemAction.blinksWhenChecked && checked
            NumberAnimation { from: 1; to: 0; duration: JamiTheme.recordBlinkDuration }
            NumberAnimation { from: 0; to: 1; duration: JamiTheme.recordBlinkDuration }
        }
    }

    // custom anchor for the tooltips
    Item {
        anchors.top: !isVertical ? parent.bottom : undefined
        anchors.topMargin: 25
        anchors.horizontalCenter: !isVertical ? parent.horizontalCenter : undefined

        anchors.right: isVertical ? parent.left : undefined
        anchors.rightMargin: isVertical ? toolTip.contentWidth / 2 + 12 : 0
        anchors.verticalCenter: isVertical ? parent.verticalCenter : undefined
        anchors.verticalCenterOffset: isVertical ? toolTip.contentHeight / 2 + 4 : 0

        MaterialToolTip {
            id: toolTip
            parent: parent
            visible: text.length > 0 && (wrapper.hovered || menu.hovered)
            text: menu.hovered ? menuAction.text : ItemAction.text
            verticalPadding: 1
            font.pointSize: 9
        }
    }

    property var menuAction: ItemAction.menuAction

    ComboBox {
        id: menu

        indicator: null

        visible: menuAction !== undefined && !BadgeCount
        anchors.horizontalCenter: parent.horizontalCenter
        width: 18
        height: width
        y: -4

        Connections {
            target: menuAction !== undefined ?
                        menuAction :
                        null
            function onTriggered() {
                itemListView.currentIndex =
                        menuAction.listModel.getCurrentIndex()
            }
        }

        contentItem: Text {
            text: "^"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "white"
        }

        background: Rectangle {
            color: menu.down ?
                       "#aaaaaa" :
                       menu.hovered ?
                           "#777777" :
                           "#444444"
            radius: 4
        }

        onActivated: menuAction.accept(index)
        model: visible ? menuAction.listModel : null
        delegate: ItemDelegate {
            id: menuItem

            width: itemListView.menuItemWidth
            height: itemListView.menuItemHeight
            background: Rectangle {
                anchors.fill: parent
                color: menuItem.down ?
                           "#c4aaaaaa" :
                           menuItem.hovered ?
                               "#c4777777" :
                               "transparent"
            }
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                ResponsiveImage {
                    source: menuItem.ListView.isCurrentItem ?
                                "qrc:/images/icons/check_box-24px.svg" :
                                "qrc:/images/icons/check_box_outline_blank-24px.svg"
                    layer.enabled: true
                    layer.effect: ColorOverlay { color: "white" }
                }
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: DeviceName
                    elide: Text.ElideRight
                    font.pointSize: 9
                    color: "white"
                }
            }
        }

        popup: Popup {
            id: itemPopup

            y: isVertical ?
                   -(implicitHeight - wrapper.height) / 2 :
                   -implicitHeight - 12
            x: isVertical ?
                   -implicitWidth - 24 :
                   -(implicitWidth - wrapper.width) / 2 - 18

            implicitWidth: contentItem.implicitWidth
            implicitHeight: contentItem.implicitHeight
            leftPadding: 0
            rightPadding: 0

            onOpened: menuAction.triggered()

            contentItem: ListView {
                id: itemListView

                property real menuItemWidth: 0
                property real menuItemHeight: 39

                orientation: ListView.Vertical
                implicitWidth: menuItemWidth
                implicitHeight: Math.min(contentHeight,
                                         menuItemHeight * 6) + 24

                ScrollIndicator.vertical: ScrollIndicator {}

                clip: true

                model: menu.delegateModel

                TextMetrics { id: itemTextMetrics }

                // recalc list width based on max item width
                onCountChanged: {
                    // Hack: use AudioDeviceModel.DeviceName role for video as well
                    var maxWidth = 0
                    for (var i = 0; i < count; ++i) {
                        var idx = menuAction.listModel.index(i, 0)
                        itemTextMetrics.text = menuAction.listModel.data(
                                    idx, AudioDeviceModel.DeviceName)
                        if (itemTextMetrics.boundingRect.width > maxWidth)
                            maxWidth = itemTextMetrics.boundingRect.width
                    }
                    // 30(icon) + 5(layout spacing) + 12(margins)
                    menuItemWidth = Math.min(256, maxWidth + 30 + 5 + 12)
                }
            }

            background: Rectangle {
                anchors.fill: parent
                radius: 5
                color: "#c4272727"
            }
        }

        layer.enabled: true
        layer.effect: DropShadow {
            z: -1
            horizontalOffset: 0
            verticalOffset: 0
            radius: 8.0
            samples: 16
            color: "#80000000"
        }
    }

    BadgeNotifier {
        id: badge

        count: BadgeCount
        anchors.horizontalCenter: parent.horizontalCenter
        width: 18
        height: width
        radius: 4
        y: -4
    }
}
