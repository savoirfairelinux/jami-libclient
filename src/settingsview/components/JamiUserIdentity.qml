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
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import Qt.labs.platform 1.1
import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth
    property bool registeredIdNeedsSet: false

    function updateAccountInfo() {
        currentRingIDText.text = SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
        registeredIdNeedsSet = (SettingsAdapter.get_CurrentAccountInfo_RegisteredName() === "")

        if(!registeredIdNeedsSet) {
            currentRegisteredID.text = SettingsAdapter.get_CurrentAccountInfo_RegisteredName()
        } else {
            currentRegisteredID.text = ""
        }
    }

    NameRegistrationDialog {
        id : nameRegistrationDialog

        onAccepted: {
            registeredIdNeedsSet = false
            currentRegisteredID.nameRegistrationState =
                    UsernameLineEdit.NameRegistrationState.BLANK
        }
    }

    // Identity
    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        Label {
            id: idLabel

            text: qsTr("Id")
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        TextField {
            id: currentRingID

            property var backgroundColor: "transparent"
            property var borderColor: "transparent"

            Layout.fillWidth: true

            font.pointSize: JamiTheme.textFontSize
            font.kerning: true
            font.bold: true

            readOnly: true
            selectByMouse: true

            text: currentRingIDText.elidedText

            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter

            background: Rectangle {
                anchors.fill: parent
                radius: 0
                border.color: currentRingID.borderColor
                border.width: 0
                color: currentRingID.backgroundColor
            }

            TextMetrics {
                id: currentRingIDText

                elide: Text.ElideRight
                elideWidth: root.width - idLabel.width -JamiTheme.preferredMarginSize*4

                text: SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        ElidedTextLabel {
            Layout.fillWidth: true

            eText: qsTr("Registered name")
            fontSize: JamiTheme.settingsFontSize
            maxWidth: width
        }

        UsernameLineEdit {
            id: currentRegisteredID

            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            Layout.preferredWidth: itemWidth
            implicitWidth: itemWidth
            wrapMode: Text.NoWrap

            placeholderText: registeredIdNeedsSet ?
                                    qsTr("Type here to register a username") : ""
            text: {
                if (!registeredIdNeedsSet)
                    return SettingsAdapter.get_CurrentAccountInfo_RegisteredName()
                else
                    return ""
            }
            readOnly: !registeredIdNeedsSet
            font.bold: !registeredIdNeedsSet

            horizontalAlignment: registeredIdNeedsSet ?
                                Text.AlignLeft :
                                Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            padding: 8
        }
    }

    MaterialButton {
        id: btnRegisterName

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        Layout.rightMargin: currentRegisteredID.width / 2 - width / 2
        Layout.preferredWidth: 120
        Layout.preferredHeight: 30

        visible: registeredIdNeedsSet &&
                    currentRegisteredID.nameRegistrationState ===
                    UsernameLineEdit.NameRegistrationState.FREE

        text: qsTr("Register")
        toolTipText: qsTr("Register the username")
        color: JamiTheme.buttonTintedGrey
        hoveredColor: JamiTheme.buttonTintedGreyHovered
        pressedColor: JamiTheme.buttonTintedGreyPressed

        onClicked: nameRegistrationDialog.openNameRegistrationDialog(currentRegisteredID.text)
    }
}