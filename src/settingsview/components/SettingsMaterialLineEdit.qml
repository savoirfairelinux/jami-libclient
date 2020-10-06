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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import Qt.labs.platform 1.1
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"
import "../../constant"

RowLayout {
    id: root

    property string titleField: ""
    property string textField: ""
    property int itemWidth
    property int wrapMode: Text.NoWrap
    property int echoMode: TextInput.Normal

    signal editFinished

    function setEnabled(status) {
        materialLineEdit.enabled = status
    }

    function setText(text) {
        root.textField = text
        materialLineEdit.text = text
    }

    Text {
        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize / 2

        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        text: titleField
        elide: Text.ElideRight
    }

    MaterialLineEdit {
        id: materialLineEdit
        Layout.alignment: Qt.AlignCenter
        Layout.preferredWidth: itemWidth

        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true

        text: textField

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: root.wrapMode
        echoMode: root.echoMode
        padding: 8

        onEditingFinished: {
            root.textField = text
            editFinished()
        }
    }
}
