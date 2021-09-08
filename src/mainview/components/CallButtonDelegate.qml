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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import net.jami.Models 1.1
import net.jami.Constants 1.1

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
        visible: ItemAction.openPopupWhenClicked !== undefined
                 && ItemAction.openPopupWhenClicked && !menu.popup.visible
        anchors.fill: wrapper
        onClicked: menu.popup.open()
    }

    background: HalfPill {
        anchors.fill: parent
        radius: type === HalfPill.None ? 0 : 5
        color: {
            if (supplimentaryBackground.visible)
                return "#c4272727"
            return wrapper.down ?
                        "#c4777777" :
                        (wrapper.hovered && !menu.hovered) ?
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
            ColorAnimation {
                duration: JamiTheme.shortFadeDuration
            }
        }
    }

    // TODO: this can be a Rectangle once multistream is done
    HalfPill {
        id: supplimentaryBackground

        visible: ItemAction.hasBg !== undefined
        color: wrapper.down ? Qt.lighter(JamiTheme.refuseRed, 1.5) :
                              wrapper.hovered && !menu.hovered ?
                                  JamiTheme.refuseRed :
                                  JamiTheme.refuseRedTransparent
        anchors.fill: parent
        radius: isLast ? 5 : width / 2
        type: isLast ? HalfPill.Right : HalfPill.None

        Behavior on color {
            ColorAnimation {
                duration: JamiTheme.shortFadeDuration
            }
        }
    }

    ResponsiveImage {
        id: icon

        // TODO: remove this when the icons are size corrected
        property real size: ItemAction.size !== undefined ? ItemAction.size : 30
        containerWidth: size
        containerHeight: size

        anchors.centerIn: parent
        source: ItemAction ? ItemAction.icon.source : ""
        color: ItemAction ? ItemAction.icon.color : null

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: ItemAction !== undefined
                     && ItemAction.blinksWhenChecked !== undefined
                     && ItemAction.blinksWhenChecked && checked
            onStopped: icon.opacity = 1
            NumberAnimation {
                from: 1
                to: 0
                duration: JamiTheme.recordBlinkDuration
            }
            NumberAnimation {
                from: 0
                to: 1
                duration: JamiTheme.recordBlinkDuration
            }
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
            text: menu.hovered ? menuAction.text : (ItemAction
                                                    !== undefined ? ItemAction.text : null)
            verticalPadding: 1
            font.pointSize: 9
        }
    }

    property var menuAction: ItemAction.menuAction

    ComboBox {
        id: menu

        indicator: null

        visible: menuAction !== undefined && !UrgentCount && menuAction.enabled

        y: isVertical ? 0 : -4
        x: isVertical ? -4 : 0
        anchors.horizontalCenter: isVertical ? undefined : parent.horizontalCenter
        anchors.verticalCenter: isVertical ? parent.verticalCenter : undefined

        width: 18
        height: width

        Connections {
            target: menuAction !== undefined ? menuAction : null
            function onTriggered() {
                if (menuAction.popupMode !== CallActionBar.ActionPopupMode.ListElement)
                    itemListView.currentIndex = menuAction.listModel.getCurrentIndex()
            }
        }

        contentItem: ResponsiveImage {
            source: isVertical ?
                        JamiResources.chevron_left_black_24dp_svg :
                        JamiResources.expand_less_24dp_svg
            color: "white"
        }

        background: Rectangle {
            color: menu.down ? "#aaaaaa" : menu.hovered ? "#777777" : "#444444"
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
                color: menuItem.down ? "#c4aaaaaa" : menuItem.hovered ? "#c4777777" : "transparent"
            }
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                ResponsiveImage {
                    source: menuAction.popupMode === CallActionBar.ActionPopupMode.ListElement ?
                                IconSource : (menuItem.ListView.isCurrentItem ?
                                                  JamiResources.check_box_24dp_svg :
                                                  JamiResources.check_box_outline_blank_24dp_svg)
                    color: "white"
                }
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: menuAction.popupMode
                          === CallActionBar.ActionPopupMode.ListElement ? Name : DeviceName
                    elide: Text.ElideRight
                    font.pointSize: 9
                    color: "white"
                }
            }
        }

        popup: Popup {
            id: itemPopup

            y: isVertical ? -(implicitHeight - wrapper.height) / 2 - 18 : -implicitHeight - 12
            x: isVertical ? -implicitWidth - 12 : -(implicitWidth - wrapper.width) / 2 - 18

            implicitWidth: contentItem.implicitWidth
            implicitHeight: contentItem.implicitHeight
            leftPadding: 0
            rightPadding: 0

            onOpened: menuAction.triggered()

            contentItem: JamiListView {
                id: itemListView

                property real menuItemWidth: 0
                property real menuItemHeight: 39

                pixelAligned: true
                orientation: ListView.Vertical
                implicitWidth: menuItemWidth
                implicitHeight: Math.min(contentHeight, menuItemHeight * 6) + 24

                model: menu.delegateModel

                TextMetrics {
                    id: itemTextMetrics

                    font.pointSize: 9
                }

                // recalc list width based on max item width
                onCountChanged: {
                    var maxWidth = 0
                    for (var i = 0; i < count; ++i) {
                        if (menuAction.popupMode === CallActionBar.ActionPopupMode.ListElement) {
                            itemTextMetrics.text = menuAction.listModel.get(i).Name
                        } else {
                            // Hack: use AudioDeviceModel.DeviceName role for video as well
                            var idx = menuAction.listModel.index(i, 0)
                            itemTextMetrics.text = menuAction.listModel.data(
                                        idx, AudioDeviceModel.DeviceName)
                        }
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
            color: "#80000000"
            transparentBorder: true
        }
    }

    BadgeNotifier {
        id: badge

        visible: count > 0
        count: UrgentCount
        anchors.horizontalCenter: parent.horizontalCenter
        width: 18
        height: width
        radius: 4
        y: -4
    }
}
