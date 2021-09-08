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
import QtQuick.Controls

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    radius: 5
    border.color: JamiTheme.tabbarBorderColor

    JamiFlickable {
        id: projectCreditsFlickable

        anchors.fill: parent

        contentHeight: projectCreditsTextArea.paintedHeight

        TextEdit {
            id: projectCreditsTextArea

            horizontalAlignment: Text.AlignHCenter

            width: projectCreditsFlickable.width

            selectByMouse: false
            readOnly: true
            wrapMode: Text.WordWrap

            font.pointSize: JamiTheme.textFontSize
            text: UtilsAdapter.getProjectCredits()
            textFormat: TextEdit.RichText

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                cursorShape: Qt.ArrowCursor
                acceptedButtons: Qt.NoButton
            }
        }
    }
}
