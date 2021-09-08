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

import QtQuick
import QtQuick.Window

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

// ScreenRubberBand as a seperate frameless window,
// is to simulate the whole screen area and provide the user
// the ability to select certain area of it.

// Typically, it is used for video screen sharing.
Window {
    id: screenRubberBandWindow

    function setAllScreensGeo() {
        var width = 0, height = 0
        var screens = Qt.application.screens
        for (var i = 0; i < screens.length; ++i) {
            width += screens[i].width
            if (height < screens[i].height)
                height = screens[i].height
        }

        screenRubberBandWindow.width = width
        screenRubberBandWindow.height = height
        screenRubberBandWindow.x = 0
        screenRubberBandWindow.y = 0
    }

    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WA_TranslucentBackground

    // Opacity with 0.7 window that will fill the entire screen,
    // provide the users to select the area that they
    // want to share.
    color: Qt.rgba(0, 0, 0, 0.7)
    // +1 so that it does not fallback to the previous screen
    x: screen.virtualX + 1
    y: screen.virtualY + 1

    screen: Qt.application.screens[0]

    // Rect for selection.
    Rectangle {
        id: recSelect

        height: 0
        width: 0

        border.color: JamiTheme.rubberBandSelectionBlue
        border.width: 1
        color: JamiTheme.rubberBandSelectionBlue
        opacity: 0.3
        visible: false
    }

    MouseArea {
        id: screenRubberBandMouseArea

        property int originalX: 0
        property int originalY: 0

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.CrossCursor

        // Geo changing for user selection.
        onPressed: {
            originalX = mouseX
            originalY = mouseY
            recSelect.x = mouseX
            recSelect.y = mouseY
            recSelect.visible = true
        }

        onMouseXChanged: {
            if (originalX - mouseX >= 0) {
                recSelect.x = mouseX
                recSelect.width = originalX - recSelect.x
            } else if (mouseX - recSelect.x > 0) {
                recSelect.width = mouseX - recSelect.x
            }
        }

        onMouseYChanged: {
            if (originalY - mouseY >= 0) {
                recSelect.y = mouseY
                recSelect.height = originalY - recSelect.y
            } else if (mouseY - recSelect.y > 0) {
                recSelect.height = mouseY - recSelect.y
            }
        }

        onReleased: {
            recSelect.visible = false
            AvAdapter.shareScreenArea(recSelect.x, recSelect.y,
                                      recSelect.width, recSelect.height)
            screenRubberBandWindow.close()
        }
    }
}
