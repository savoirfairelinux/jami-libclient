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
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../commoncomponents"

BaseDialog {
    id: root

    property int profileType: SettingsAdapter.getCurrentAccount_Profile_Info_Type()

    property bool isSIP: {
        switch (profileType) {
        case Profile.Type.SIP:
            return true;
        default:
            return false;
        }
    }

    signal accepted

    function openDialog() {
        profileType = SettingsAdapter.getCurrentAccount_Profile_Info_Type()
        labelBestId.text = SettingsAdapter.getAccountBestName()
        labelAccountHash.text = SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
        open()
    }

    title: JamiStrings.deleteAccount

    contentItem: Rectangle {
        id: deleteAccountContentRect

        implicitWidth: JamiTheme.preferredDialogWidth
        implicitHeight: JamiTheme.preferredDialogHeight
        color: JamiTheme.secondaryBackgroundColor

        ColumnLayout {
            anchors.centerIn: parent
            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize

            Label {
                id: labelDeletion

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: deleteAccountContentRect.width - JamiTheme.preferredMarginSize * 2

                color: JamiTheme.textColor
                text: JamiStrings.confirmDeleteQuestion

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
            }

            Label {
                id: labelBestId

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: deleteAccountContentRect.width - JamiTheme.preferredMarginSize * 2

                color: JamiTheme.textColor
                text: SettingsAdapter.getAccountBestName()

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                font.bold: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
            }

            Label {
                id: labelAccountHash

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: deleteAccountContentRect.width - JamiTheme.preferredMarginSize * 2

                color: JamiTheme.textColor
                text: SettingsAdapter.getCurrentAccount_Profile_Info_Uri()

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
            }

            Label {
                id: labelWarning

                visible: !isSIP

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: deleteAccountContentRect.width - JamiTheme.preferredMarginSize * 2

                text: JamiStrings.deleteAccountInfos

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap

                color: "red"
            }

            RowLayout {
                spacing: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter

                MaterialButton {
                    id: btnDelete

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    color: JamiTheme.buttonTintedRed
                    hoveredColor: JamiTheme.buttonTintedRedHovered
                    pressedColor: JamiTheme.buttonTintedRedPressed
                    outlined: true

                    text: qsTr("Delete")

                    onClicked: {
                        AccountAdapter.deleteCurrentAccount()
                        accepted()
                        close()
                    }
                }

                MaterialButton {
                    id: btnCancel

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    outlined: true

                    text: qsTr("Cancel")

                    onClicked: {
                        close()
                    }
                }
            }
        }
    }
}
