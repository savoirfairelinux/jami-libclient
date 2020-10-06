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
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import "../../constant"
import "../../commoncomponents"

// An independent widget that keeps the password's textfields, including password field and confirm password field
GridLayout {
    id: root

    property alias text_passwordEditAlias: passwordEdit.text
    property alias text_confirmPasswordEditAlias: confirmPasswordEdit.text
    property alias state_passwordStatusLabelAlias: passwordStatusLabel.passwordStatusState

    property bool visibleCollapsble: false

    function clearAllTextFields() {
        passwordEdit.clear()
        confirmPasswordEdit.clear()
    }

    visible: visibleCollapsble
    Layout.fillWidth: true
    rowSpacing: 6
    columnSpacing: 6

    rows: 2
    columns: 2

    Layout.leftMargin: 32

    MaterialLineEdit {
        id: passwordEdit

        visible: visibleCollapsble

        Layout.row: 0
        Layout.column: 0

        fieldLayoutWidth: 261

        Layout.alignment: Qt.AlignHCenter

        selectByMouse: true
        echoMode: TextInput.Password
        placeholderText: qsTr("Password")
        font.pointSize: 10
        font.kerning: true
    }

    Item {
        Layout.row: 0
        Layout.column: 1

        Layout.maximumWidth: 32
        Layout.preferredWidth: 32
        Layout.minimumWidth: 32

        Layout.maximumHeight: 30
        Layout.preferredHeight: 30
        Layout.minimumHeight: 30
    }

    MaterialLineEdit {
        id: confirmPasswordEdit

        visible: visibleCollapsble

        Layout.row: 1
        Layout.column: 0

        fieldLayoutWidth: 261

        Layout.alignment: Qt.AlignHCenter

        selectByMouse: true
        echoMode: TextInput.Password
        placeholderText: qsTr("Confirm Password")
        font.pointSize: 10
        font.kerning: true
    }

    Label {
        id: passwordStatusLabel

        visible: visibleCollapsble

        Layout.row: 1
        Layout.column: 1

        Layout.maximumWidth: 32
        Layout.preferredWidth: 32
        Layout.minimumWidth: 32

        Layout.maximumHeight: 30
        Layout.preferredHeight: 30
        Layout.minimumHeight: 30

        Layout.alignment: Qt.AlignRight

        property string passwordStatusState: "Hide"

        background: {
            switch (passwordStatusState) {
            case "Hide":
                return Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
color: \"transparent\"; }", passwordStatusLabel)
            case "Fail":
                return Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
Image{
anchors.fill: parent;
source: \"image://tintedPixmap/\"+ (\"qrc:/images/icons/baseline-close-24px.svg\").replace(\"qrc:/images/icons/\",\"\") + \"+\" + JamiTheme.red_;
mipmap: true;}
}", passwordStatusLabel)
            case "Success":
                return Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
Image {
anchors.fill: parent;
source: \"image://tintedPixmap/\"+ (\"qrc:/images/icons/baseline-done-24px.svg\").replace(\"qrc:/images/icons/\",\"\") + \"+\" + JamiTheme.presenceGreen_;
mipmap: true;}
}", passwordStatusLabel)
            }
        }
    }
}
