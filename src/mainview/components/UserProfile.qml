
/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
import net.jami.Models 1.0

import "../../commoncomponents"

Dialog {
    id: userProfileDialog

    property string responsibleConvUid: ""
    property string contactPicBase64: ""
    property string aliasText: ""
    property string registeredNameText: ""
    property string idText: ""

    modal: true


    /*
     * Content height + margin.
     */
    contentHeight: userProfileDialogUpperPartColumnLayout.implicitHeight + 30


    /*
     * Fake focus to make sure that text edit lose focus on close.
     */
    FocusScope {
        id: fakeFocusTextEdit
    }

    ColumnLayout {
        id: userProfileDialogUpperPartColumnLayout

        anchors.centerIn: parent

        spacing: 15

        Image {
            id: contactImage

            Layout.alignment: Qt.AlignCenter

            width: 150
            height: 150

            fillMode: Image.PreserveAspectFit
            mipmap: true
        }


        /*
         * Visible when user alias is not empty or equals to id.
         */
        Text {
            id: contactAlias

            Layout.alignment: Qt.AlignCenter

            font.pointSize: JamiTheme.textFontSize
            text: textMetricsContactAliasText.elidedText
            visible: aliasText ? (aliasText === idText ? false : true) : false
            TextMetrics {
                id: textMetricsContactAliasText
                font: contactAlias.font
                text: aliasText
                elideWidth: userProfileDialog.width - 30
                elide: Qt.ElideMiddle
            }
        }


        /*
         * Visible when user name is not empty or equals to alias.
         */
        Text {
            id: contactDisplayName

            Layout.alignment: Qt.AlignCenter

            font.pointSize: JamiTheme.textFontSize - 1
            text: textMetricsContactDisplayNameText.elidedText
            visible: registeredNameText ? (registeredNameText === aliasText ? false : true) : false
            color: JamiTheme.faddedFontColor

            TextMetrics {
                id: textMetricsContactDisplayNameText
                font: contactDisplayName.font
                text: registeredNameText
                elideWidth: userProfileDialog.width - 30
                elide: Qt.ElideMiddle
            }
        }

        TextEdit {
            id: contactId

            Layout.alignment: Qt.AlignCenter

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            selectByMouse: true
            readOnly: true
            font.pointSize: JamiTheme.textFontSize - 1
            text: textMetricsContactIdText.elidedText

            TextMetrics {
                id: textMetricsContactIdText
                font: contactId.font
                text: idText
                elideWidth: userProfileDialog.width - 30
                elide: Qt.ElideMiddle
            }
        }

        Image {
            id: contactQrImage

            Layout.alignment: Qt.AlignBottom | Qt.AlignCenter

            width: 150
            height: 150

            fillMode: Image.PreserveAspectFit
            sourceSize.width: 150
            sourceSize.height: 150
            mipmap: true
        }
    }

    background: Rectangle {
        border.width: 0
        radius: 10
    }

    onClosed: {
        contactId.deselect()
        fakeFocusTextEdit.focus = true
    }

    onResponsibleConvUidChanged: {
        if (responsibleConvUid !== "")
            contactQrImage.source = "image://qrImage/contact_" + responsibleConvUid
    }

    onContactPicBase64Changed: {
        if (contactPicBase64 !== "")
            contactImage.source = "data:image/png;base64," + contactPicBase64
    }
}
