/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQml 2.14

import net.jami.Constants 1.1

ComboBox {
    id: root

    property string tooltipText
    property string comboBoxBackgroundColor: JamiTheme.editBackgroundColor
    property string placeholderText

    displayText: currentIndex !== -1 ?
                     currentText :
                     (placeholderText !== "" ?
                          placeholderText :
                          JamiStrings.notAvailable)

    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.visible: hovered && (tooltipText.length > 0)
    ToolTip.text: tooltipText

    delegate: ItemDelegate {
        width: root.width
        contentItem: Text {
            text: {
                if (index >= 0) {
                    var currentItem = root.delegateModel.items.get(index)
                    return currentItem.model[root.textRole].toString()
                }
                return ""
            }
            color: JamiTheme.textColor
            font: root.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: root.highlightedIndex === index
        background: Rectangle {
            color: highlighted? JamiTheme.selectedColor : JamiTheme.editBackgroundColor
        }
    }

    indicator: Canvas {
        id: canvas
        x: root.width - width - root.rightPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: root
            function onPressedChanged(){
                canvas.requestPaint()
            }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = root.pressed ? JamiTheme.pressColor : JamiTheme.textColor;
            context.fill();
        }
    }

    contentItem: Text {
        leftPadding: 10
        rightPadding: root.indicator.width + leftPadding

        text: root.displayText
        font: root.font
        color: JamiTheme.textColor
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        color: root.comboBoxBackgroundColor
        implicitWidth: 120
        implicitHeight: 40
        border.color: root.comboBoxBackgroundColor
        border.width: root.visualFocus ? 2 : 1
        radius: 2
    }

    popup: Popup {
        y: root.height - 1
        width: root.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            id: listView
            clip: true
            implicitHeight: contentHeight
            model: root.delegateModel
            currentIndex: root.highlightedIndex

            ScrollBar.vertical: ScrollBar { }
        }

        background: Rectangle {
            color: JamiTheme.editBackgroundColor
            border.color: "gray"
            radius: 2
        }
    }
}
