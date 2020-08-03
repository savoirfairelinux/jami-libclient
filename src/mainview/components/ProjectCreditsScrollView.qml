
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

ScrollView {
    id: projectCreditsScrollView

    anchors.fill: parent

    clip: true

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    TextEdit {
        id: projectCreditsTextArea

        horizontalAlignment: Text.AlignHCenter

        width: projectCreditsScrollView.width

        selectByMouse: false
        readOnly: true
        wrapMode: Text.WordWrap

        font.pointSize: JamiTheme.textFontSize - 3
        text: ClientWrapper.utilsAdaptor.getProjectCredits()
        textFormat: TextEdit.RichText

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            cursorShape: Qt.ArrowCursor
            acceptedButtons: Qt.NoButton
        }
    }

    background: Rectangle {
        id: projectCreditsScrollViewBackground

        radius: 5
        border.color: JamiTheme.tabbarBorderColor
    }
}
