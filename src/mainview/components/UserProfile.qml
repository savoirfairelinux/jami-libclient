
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

    property int preferredImgSize: 80
    modal: true

    // Content height + margin.
    contentHeight: userProfileDialogLayout.implicitHeight + 60
    contentWidth: userProfileDialogLayout.implicitWidth + 60

    // Fake focus to make sure that text edit lose focus on close.
    FocusScope {
        id: fakeFocusTextEdit
    }

    contentItem: GridLayout {

        id: userProfileDialogLayout
        columns: 2
        rowSpacing: 10
        columnSpacing: 10

        Image {
            id: contactImage

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: 130

            sourceSize.width: preferredImgSize
            sourceSize.height: preferredImgSize

            fillMode: Image.PreserveAspectFit
            mipmap: true
        }

        // Visible when user alias is not empty or equals to id.
        Text {
            id: contactAlias

            Layout.alignment: Qt.AlignLeft

            font.pointSize: JamiTheme.titleFontSize
            text: textMetricsContactAliasText.elidedText
            visible: aliasText ? (aliasText === idText ? false : true) : false

            TextMetrics {
                id: textMetricsContactAliasText
                font: contactAlias.font
                text: aliasText
                elideWidth: userProfileDialog.width-160
                elide: Qt.ElideMiddle
            }
        }

        Item {
            Layout.columnSpan: 2
            height: 20
        }

        Text {
            Layout.alignment: Qt.AlignRight
            font.pointSize: JamiTheme.menuFontSize
            text: qsTr("Informations")
        }

        Item { Layout.fillWidth: true }

        Text {
            Layout.alignment: Qt.AlignRight
            font.pointSize: JamiTheme.textFontSize
            text: qsTr("Username")
            visible: registeredNameText ? (registeredNameText === idText ? false : true) : false
            color: JamiTheme.faddedFontColor
        }

        // Visible when user name is not empty or equals to alias.
        Text {
            id: contactDisplayName

            Layout.alignment: Qt.AlignLeft

            font.pointSize: JamiTheme.textFontSize
            text: textMetricsContactDisplayNameText.elidedText
            visible: registeredNameText ? (registeredNameText === idText ? false : true) : false

            TextMetrics {
                id: textMetricsContactDisplayNameText
                font: contactDisplayName.font
                text: registeredNameText
                elideWidth: userProfileDialog.width-160
                elide: Qt.ElideMiddle
            }
        }

        Text {
            Layout.alignment: Qt.AlignRight
            font.pointSize: JamiTheme.textFontSize
            text: qsTr("ID")
            color: JamiTheme.faddedFontColor
        }

        TextEdit {
            id: contactId

            Layout.alignment: Qt.AlignLeft

            selectByMouse: true
            readOnly: true
            font.pointSize: JamiTheme.textFontSize
            text: textMetricsContactIdText.elidedText

            TextMetrics {
                id: textMetricsContactIdText
                font: contactId.font
                text: idText
                elideWidth: userProfileDialog.width-160
                elide: Qt.ElideMiddle
            }
        }

        Text {
            Layout.alignment: Qt.AlignRight
            font.pointSize: JamiTheme.textFontSize
            text: qsTr("QR Code")
            color: JamiTheme.faddedFontColor
        }

        Image {
            id: contactQrImage

            Layout.alignment: Qt.AlignBottom | Qt.AlignLeft

            fillMode: Image.PreserveAspectFit
            sourceSize.width: preferredImgSize
            sourceSize.height: preferredImgSize
            mipmap: true
        }

        Item { height: 20 }

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
