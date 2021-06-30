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

import net.jami.Constants 1.0

import "../commoncomponents/contextmenu"

Flickable {
    id: root

    property alias text: textArea.text

    function insertText(text) {
        textArea.insert(textArea.cursorPosition, text)
    }

    LineEditContextMenu {
        id: textAreaContextMenu

        lineEditObj: textArea
    }

    ScrollBar.vertical: ScrollBar {
        policy: contentHeight > height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
    }

    contentWidth: width
    contentHeight: textArea.implicitHeight

    interactive: true
    clip: true

    function ensureVisible(r) {
        if (contentY >= r.y)
            contentY = r.y
        else if (contentY + height <= r.y + r.height)
            contentY = r.y + r.height - height
    }

    TextArea.flickable: TextArea {
        id: textArea

        padding: 0

        verticalAlignment: TextEdit.AlignVCenter

        font.pointSize: JamiTheme.textFontSize + 2
        font.hintingPreference: Font.PreferNoHinting

        color: JamiTheme.textColor
        renderType: Text.NativeRendering
        wrapMode: TextEdit.Wrap
        overwriteMode: true
        selectByMouse: true
        selectionColor: JamiTheme.placeHolderTextFontColor
        textFormat: TextEdit.PlainText
        placeholderTextColor: JamiTheme.placeHolderTextFontColor

        cursorDelegate: Rectangle {
            visible: textArea.cursorVisible
            color: JamiTheme.textColor
            width: 1

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                running: visible

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
        background: Rectangle {
            border.width: 0
            color: JamiTheme.transparentColor
        }

        onReleased: {
            if (event.button == Qt.RightButton)
                textAreaContextMenu.openMenuAt(event)
        }

        onCursorRectangleChanged: root.ensureVisible(cursorRectangle)
    }
}
