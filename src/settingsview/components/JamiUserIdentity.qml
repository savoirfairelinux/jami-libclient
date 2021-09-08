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
import QtQuick.Controls

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    NameRegistrationDialog {
        id : nameRegistrationDialog

        onAccepted: currentRegisteredID.nameRegistrationState =
                    UsernameLineEdit.NameRegistrationState.BLANK
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

                text: CurrentAccount.uri
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

            padding: 8
            horizontalAlignment: CurrentAccount.registeredName === "" ? Text.AlignLeft :
                                                                        Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.NoWrap
            placeholderText: CurrentAccount.registeredName === "" ?
                                 JamiStrings.registerAUsername : ""
            text: CurrentAccount.registeredName
            readOnly: CurrentAccount.registeredName !== ""
            font.bold: CurrentAccount.registeredName !== ""
            loseFocusWhenEnterPressed: btnRegisterName.visible

            onAccepted: {
                if (btnRegisterName.visible)
                    btnRegisterName.clicked()
            }
        }
    }

    MaterialButton {
        id: btnRegisterName

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        Layout.rightMargin: currentRegisteredID.width / 2 - width / 2

        preferredWidth: 120
        preferredHeight: 30

        visible: CurrentAccount.registeredName === "" &&
                 currentRegisteredID.nameRegistrationState ===
                 UsernameLineEdit.NameRegistrationState.FREE

        text: JamiStrings.register
        toolTipText: JamiStrings.registerUsername
        color: JamiTheme.buttonTintedGrey
        hoveredColor: JamiTheme.buttonTintedGreyHovered
        pressedColor: JamiTheme.buttonTintedGreyPressed

        onClicked: nameRegistrationDialog.openNameRegistrationDialog(currentRegisteredID.text)
    }
}
