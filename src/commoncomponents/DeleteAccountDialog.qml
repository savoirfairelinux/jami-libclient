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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../commoncomponents"

BaseModalDialog {
    id: root

    property bool isSIP: {
        switch (CurrentAccount.type) {
        case Profile.Type.SIP:
            return true;
        default:
            return false;
        }
    }

    signal accepted

    title: JamiStrings.deleteAccount

    width: JamiTheme.preferredDialogWidth
    height: JamiTheme.preferredDialogHeight

    popupContent: ColumnLayout {
        id: deleteAccountContentColumnLayout

        Label {
            id: labelDeletion

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: deleteAccountContentColumnLayout.width -
                                   JamiTheme.preferredMarginSize * 2

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
            Layout.preferredWidth: deleteAccountContentColumnLayout.width -
                                   JamiTheme.preferredMarginSize * 2

            color: JamiTheme.textColor
            text: CurrentAccount.bestName

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
            Layout.preferredWidth: deleteAccountContentColumnLayout.width -
                                   JamiTheme.preferredMarginSize * 2

            color: JamiTheme.textColor
            text: CurrentAccount.uri

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
            Layout.preferredWidth: deleteAccountContentColumnLayout.width -
                                   JamiTheme.preferredMarginSize * 2

            text: JamiStrings.deleteAccountInfos

            font.pointSize: JamiTheme.textFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap

            color: JamiTheme.redColor
        }

        RowLayout {
            spacing: 16
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter

            MaterialButton {
                id: btnDelete

                Layout.alignment: Qt.AlignHCenter

                preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                preferredHeight: JamiTheme.preferredFieldHeight

                color: JamiTheme.buttonTintedRed
                hoveredColor: JamiTheme.buttonTintedRedHovered
                pressedColor: JamiTheme.buttonTintedRedPressed
                outlined: true

                text: JamiStrings.optionDelete

                onClicked: {
                    AccountAdapter.deleteCurrentAccount()
                    accepted()
                    close()
                }
            }

            MaterialButton {
                id: btnCancel

                Layout.alignment: Qt.AlignHCenter

                preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                preferredHeight: JamiTheme.preferredFieldHeight

                color: JamiTheme.buttonTintedBlack
                hoveredColor: JamiTheme.buttonTintedBlackHovered
                pressedColor: JamiTheme.buttonTintedBlackPressed
                outlined: true

                text: JamiStrings.optionCancel

                onClicked: close()
            }
        }
    }
}
