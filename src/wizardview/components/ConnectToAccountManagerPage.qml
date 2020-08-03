/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14

import "../../constant"
import "../../commoncomponents"

ColumnLayout {
    property alias text_usernameManagerEditAlias: usernameManagerEdit.text
    property alias text_passwordManagerEditAlias: passwordManagerEdit.text
    property alias text_accountManagerEditAlias: accountManagerEdit.text

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        usernameManagerEdit.clear()
        passwordManagerEdit.clear()
        accountManagerEdit.clear()
    }

    Layout.fillWidth: true
    Layout.fillHeight: true

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    ColumnLayout {
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true

        spacing: 12

        Label {
            id: signInLabel

            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: 256
            Layout.preferredHeight: 21
            text: qsTr("Sign in")
            font.pointSize: 13
            font.kerning: true
        }

        InfoLineEdit {
            id: usernameManagerEdit

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            placeholderText: qsTr("Username")
        }

        InfoLineEdit {
            id: passwordManagerEdit

            Layout.alignment: Qt.AlignHCenter
            selectByMouse: true
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
        }

        InfoLineEdit {
            id: accountManagerEdit

            Layout.alignment: Qt.AlignHCenter

            selectByMouse: true
            placeholderText: qsTr("Account Manager")
        }
    }

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
