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

import QtQuick
import QtQuick.Layouts

import net.jami.Constants 1.1

import "../../commoncomponents"

BaseModalDialog {
    id: root

    property string convId
    property string aliasText
    property string registeredNameText
    property string idText
    property bool isSwarm

    property int preferredImgSize: 80

    width: 480
    height: 480

    popupContent: Rectangle {
        id: userProfileContentRect

        color: JamiTheme.backgroundColor
        radius: JamiTheme.modalPopupRadius

        GridLayout {
            id: userProfileDialogLayout

            anchors.centerIn: parent
            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize

            columns: 2
            rows: 6
            rowSpacing: 16
            columnSpacing: 24

            ConversationAvatar {
                id: contactImage

                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: preferredImgSize
                Layout.preferredHeight: preferredImgSize

                imageId: convId
                showPresenceIndicator: false
            }

            // Visible when user alias is not empty or equals to id.
            MaterialLineEdit {
                id: contactAlias

                Layout.alignment: Qt.AlignLeft

                font.pointSize: JamiTheme.titleFontSize
                font.kerning: true
                color: JamiTheme.textColor
                visible: aliasText ? (aliasText === idText ? false : true) : false

                padding: 0
                readOnly: true
                selectByMouse: true

                wrapMode: Text.NoWrap
                text: textMetricsContactAliasText.elidedText

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                TextMetrics {
                    id: textMetricsContactAliasText
                    font: contactAlias.font
                    text: aliasText
                    elideWidth: userProfileContentRect.width - 200
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
                text: JamiStrings.information
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
            MaterialLineEdit {
                id: contactDisplayName

                Layout.alignment: Qt.AlignLeft

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                color: JamiTheme.textColor
                visible: registeredNameText ? (registeredNameText === idText ? false : true) : false

                padding: 0
                readOnly: true
                selectByMouse: true

                wrapMode: Text.NoWrap
                text: textMetricsContactDisplayNameText.elidedText

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                TextMetrics {
                    id: textMetricsContactDisplayNameText
                    font: contactDisplayName.font
                    text: registeredNameText
                    elideWidth: userProfileContentRect.width - 200
                    elide: Qt.ElideMiddle
                }
            }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pointSize: JamiTheme.textFontSize
                text: JamiStrings.identifier
                color: JamiTheme.faddedFontColor
            }

            MaterialLineEdit {
                id: contactId

                Layout.alignment: Qt.AlignLeft

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                color: JamiTheme.textColor

                padding: 0
                readOnly: true
                selectByMouse: true

                wrapMode: Text.NoWrap
                text: textMetricsContactIdText.elidedText

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                TextMetrics {
                    id: textMetricsContactIdText
                    font: contactId.font
                    text: idText
                    elideWidth: userProfileContentRect.width - 200
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
                mipmap: false
                smooth: false

                source: convId !== "" ?
                            "image://qrImage/contact_" + convId :
                            ""
            }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pointSize: JamiTheme.textFontSize
                text: JamiStrings.isSwarm
                color: JamiTheme.faddedFontColor
            }

            Text {
                Layout.alignment: Qt.AlignLeft
                font.pointSize: JamiTheme.textFontSize
                text: isSwarm? JamiStrings.trueStr : JamiStrings.falseStr
                color: JamiTheme.faddedFontColor
            }

            MaterialButton {
                id: btnClose

                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter

                preferredWidth: JamiTheme.preferredFieldWidth / 2
                preferredHeight: JamiTheme.preferredFieldHeight

                color: JamiTheme.buttonTintedBlack
                hoveredColor: JamiTheme.buttonTintedBlackHovered
                pressedColor: JamiTheme.buttonTintedBlackPressed
                outlined: true

                text: JamiStrings.close

                onClicked: close()
            }
        }
    }
}
