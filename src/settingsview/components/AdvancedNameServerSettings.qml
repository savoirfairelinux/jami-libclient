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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    Text {
        Layout.fillWidth: true
        Layout.rightMargin: JamiTheme.preferredMarginSize / 2

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        text: qsTr("Name Server")
        color: JamiTheme.textColor
        elide: Text.ElideRight
    }

    SettingsMaterialLineEdit {
        id: lineEditNameServer

        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        itemWidth: root.itemWidth
        titleField: qsTr("Address")

        textField: CurrentAccount.uri_RingNS

        onEditFinished: CurrentAccount.uri_RingNS = textField
    }
}
