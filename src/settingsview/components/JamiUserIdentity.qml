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
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

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
    Row {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        spacing: width - idLabel.width - currentRingID.width

        Label {
            id: idLabel

            anchors.verticalCenter: parent.verticalCenter

            text: JamiStrings.identifier
            color: JamiTheme.textColor
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        MaterialLineEdit {
            id: currentRingID

            anchors.verticalCenter: parent.verticalCenter

            width: parent.width - idLabel.width
                   - JamiTheme.preferredMarginSize
            height: JamiTheme.preferredFieldHeight

            font.pointSize: JamiTheme.textFontSize
            font.kerning: true
            font.bold: true

            padding: 0
            readOnly: true
            selectByMouse: true

            wrapMode: Text.NoWrap
            text: currentRingIDText.elidedText
            color: JamiTheme.textColor

            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter

            TextMetrics {
                id: currentRingIDText

                font: currentRingID.font
                elide: Text.ElideRight
                elideWidth: root.width - idLabel.width -
                            JamiTheme.preferredMarginSize * 4

                text: SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
            }
        }
    }

    Row {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        spacing: width - lblRegisteredName.width - currentRegisteredID.width

        Label {
            id: lblRegisteredName

            anchors.verticalCenter: parent.verticalCenter

            text: JamiStrings.username
            color: JamiTheme.textColor
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        UsernameLineEdit {
            id: currentRegisteredID

            anchors.verticalCenter: parent.verticalCenter

            height: JamiTheme.preferredFieldHeight
            width: {
                var maximumWidth = parent.width - lblRegisteredName.width
                        - JamiTheme.preferredMarginSize
                return fieldLayoutWidth < maximumWidth ?
                            fieldLayoutWidth : maximumWidth
            }

            wrapMode: Text.NoWrap
            placeholderText: registeredIdNeedsSet ?
                                    JamiStrings.registerAUsername : ""
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
        toolTipText: JamiStrings.registerUsername
        color: JamiTheme.buttonTintedGrey
        hoveredColor: JamiTheme.buttonTintedGreyHovered
        pressedColor: JamiTheme.buttonTintedGreyPressed

        onClicked: nameRegistrationDialog.openNameRegistrationDialog(currentRegisteredID.text)
    }
}
