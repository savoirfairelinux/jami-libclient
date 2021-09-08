/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

RowLayout {
    id: root

    property alias titleField: title.text
    property alias textField: materialLineEdit.text
    property alias enabled: materialLineEdit.enabled

    property int itemWidth
    property int wrapMode: Text.NoWrap
    property int echoMode: TextInput.Normal

    signal editFinished

    Text {
        id: title

        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize / 2

        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        color: JamiTheme.textColor
        elide: Text.ElideRight
    }

    MaterialLineEdit {
        id: materialLineEdit

        Layout.alignment: Qt.AlignCenter
        Layout.preferredWidth: itemWidth

        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true

        padding: 8
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        loseFocusWhenEnterPressed: true
        wrapMode: root.wrapMode
        echoMode: root.echoMode

        onEditingFinished: editFinished()
    }
}
