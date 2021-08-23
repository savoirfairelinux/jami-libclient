/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

import net.jami.Constants 1.1

Item {
    property alias text: shortcutText.text
    Rectangle{
        id: keyRect
        width: t_metrics.tightBoundingRect.width + 10
        height: t_metrics.tightBoundingRect.height + 10
        color: JamiTheme.buttonTintedGrey
        radius: 5
        anchors.centerIn: parent
        Text {
            id : shortcutText
            anchors.centerIn: parent
            anchors.leftMargin: 10
            font.family: "Arial"
            font.pointSize: 12
            color: JamiTheme.whiteColor
        }
        TextMetrics {
            id:     t_metrics
            font:   shortcutText.font
            text:   shortcutText.text
        }
    }
}
