/*
 * Copyright (C) 2020-2022 Savoir-faire Linux Inc.
 * Author: Albert Babí <albert.babig@savoirfairelinux.com>
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

import net.jami.Constants 1.1

TextEdit {
    id: root

    property string eText : ""
    property int maxWidth: 100
    property int fontSize: JamiTheme.textFontSize
    property int textWidth: 0

    font.pointSize: fontSize
    font.kerning: true

    text: elided.elidedText

    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter
    color: JamiTheme.textColor

    readOnly: true
    selectByMouse: true

    TextMetrics {
        id: elided

        font: root.font
        elide: Text.ElideRight
        elideWidth: maxWidth
        text: eText

        onTextChanged: {
            textWidth = elided.boundingRect.width
        }
    }
}
