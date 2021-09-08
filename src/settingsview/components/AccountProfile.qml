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

ColumnLayout {
    id: root

    Connections {
        target: settingsViewRect

        function onStopBooth() {
            stopBooth()
        }
    }

    function stopBooth() {
        currentAccountAvatar.stopBooth()
    }

    Text {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        text: qsTr("Profile")
        elide: Text.ElideRight

        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true
        color: JamiTheme.textColor

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    PhotoboothView {
        id: currentAccountAvatar

        Layout.alignment: Qt.AlignCenter

        imageId: LRCInstance.currentAccountId
        avatarSize: 180
    }

    MaterialLineEdit {
        id: displayNameLineEdit

        Layout.alignment: Qt.AlignCenter
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.preferredWidth: JamiTheme.preferredFieldWidth

        font.pointSize: JamiTheme.textFontSize
        font.kerning: true
        text: CurrentAccount.alias

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        padding: 8

        loseFocusWhenEnterPressed: true

        onEditingFinished: AccountAdapter.setCurrAccDisplayName(text)
    }
}
