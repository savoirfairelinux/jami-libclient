/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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

import "../../constant"
import "../../commoncomponents"

BaseDialog {
    id: root

    property string responsibleConvUid: ""
    property string contactImageUid: ""
    property string aliasText: ""
    property string registeredNameText: ""
    property string idText: ""

    property int preferredImgSize: 80

    contentItem: Rectangle {
        id: userProfileContentRect

        implicitWidth: 480
        implicitHeight: 400

        color: JamiTheme.backgroundColor

        GridLayout {
            id: userProfileDialogLayout

            anchors.centerIn: parent
            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize

            columns: 2
            rows: 6
            rowSpacing: 16
            columnSpacing: 24

            AvatarImage {
                id: contactImage

                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: preferredImgSize
                Layout.preferredHeight: preferredImgSize

                sourceSize.width: preferredImgSize
                sourceSize.height: preferredImgSize

                mode: AvatarImage.Mode.FromConvUid
                showPresenceIndicator: false
            }

            // Visible when user alias is not empty or equals to id.
            Text {
                id: contactAlias

                Layout.alignment: Qt.AlignLeft

                font.pointSize: JamiTheme.titleFontSize
                text: textMetricsContactAliasText.elidedText
                color: JamiTheme.textColor
                visible: aliasText ? (aliasText === idText ? false : true) : false

                TextMetrics {
                    id: textMetricsContactAliasText
                    font: contactAlias.font
                    text: aliasText
                    elideWidth: userProfileContentRect.width-200
                    elide: Qt.ElideMiddle
                }
            }

            Item {
                Layout.columnSpan: 2
                height: 8
            }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pointSize: JamiTheme.menuFontSize
                text: qsTr("Information")
                color: JamiTheme.textColor
            }

            Item { Layout.fillWidth: true }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pointSize: JamiTheme.textFontSize
                text: JamiStrings.username
                visible: registeredNameText ? (registeredNameText === idText ? false : true) : false
                color: JamiTheme.faddedFontColor
            }

            // Visible when user name is not empty or equals to alias.
            Text {
                id: contactDisplayName

                Layout.alignment: Qt.AlignLeft

                font.pointSize: JamiTheme.textFontSize
                text: textMetricsContactDisplayNameText.elidedText
                color: JamiTheme.textColor
                visible: registeredNameText ? (registeredNameText === idText ? false : true) : false

                TextMetrics {
                    id: textMetricsContactDisplayNameText
                    font: contactDisplayName.font
                    text: registeredNameText
                    elideWidth: userProfileContentRect.width-200
                    elide: Qt.ElideMiddle
                }
            }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pointSize: JamiTheme.textFontSize
                text: JamiStrings.identifier
                color: JamiTheme.faddedFontColor
            }

            TextEdit {
                id: contactId

                Layout.alignment: Qt.AlignLeft

                selectByMouse: true
                readOnly: true
                font.pointSize: JamiTheme.textFontSize
                text: textMetricsContactIdText.elidedText
                color: JamiTheme.textColor

                TextMetrics {
                    id: textMetricsContactIdText
                    font: contactId.font
                    text: idText
                    elideWidth: userProfileContentRect.width-200
                    elide: Qt.ElideMiddle
                }
            }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pointSize: JamiTheme.textFontSize
                text: JamiStrings.qrCode
                color: JamiTheme.faddedFontColor
            }

            Image {
                id: contactQrImage

                Layout.alignment: Qt.AlignLeft

                fillMode: Image.PreserveAspectFit
                sourceSize.width: preferredImgSize
                sourceSize.height: preferredImgSize
                mipmap: true
            }

            MaterialButton {
                id: btnClose

                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: JamiTheme.preferredFieldWidth / 2
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                color: JamiTheme.buttonTintedBlack
                hoveredColor: JamiTheme.buttonTintedBlackHovered
                pressedColor: JamiTheme.buttonTintedBlackPressed
                outlined: true

                text: JamiStrings.close

                onClicked: {
                    close()
                }
            }
        }
    }

    onResponsibleConvUidChanged: {
        if (responsibleConvUid !== "")
            contactQrImage.source = "image://qrImage/contact_" + responsibleConvUid
    }

    onContactImageUidChanged: contactImage.updateImage(contactImageUid)
}
