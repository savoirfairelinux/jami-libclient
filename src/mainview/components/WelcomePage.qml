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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    anchors.fill: parent
    color: JamiTheme.secondaryBackgroundColor

    ColumnLayout {
        id: welcomePageColumnLayout

        anchors.centerIn: parent

        width: Math.max(mainViewStackPreferredWidth, root.width - 100)
        height: parent.height

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: welcomePageColumnLayout.width
            Layout.preferredHeight: implicitHeight
            Layout.topMargin: JamiTheme.preferredMarginSize

            ResponsiveImage {
                id: jamiLogoImage

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: Math.min(welcomePageColumnLayout.width, 330)
                Layout.preferredHeight: Math.min(welcomePageColumnLayout.width / 3, 110)
                Layout.bottomMargin: 10

                source: JamiTheme.darkTheme ?
                            JamiResources.logo_jami_standard_coul_white_svg :
                            JamiResources.logo_jami_standard_coul_svg
            }

            Label {
                id: jamiIntroText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomePageColumnLayout.width
                Layout.preferredHeight: 80
                Layout.bottomMargin: 5

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize + 1

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: JamiStrings.description
                color: JamiTheme.textColor
            }

            Label {
                id: jamiShareWithFriendText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomePageColumnLayout.width
                Layout.preferredHeight: 50

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                visible: LRCInstance.currentAccountType === Profile.Type.JAMI

                text: JamiStrings.shareInvite
                color: JamiTheme.faddedFontColor
            }

            Rectangle {
                id: jamiRegisteredNameRect

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomePageColumnLayout.width
                Layout.preferredHeight: 65

                color: JamiTheme.secondaryBackgroundColor

                visible: LRCInstance.currentAccountType === Profile.Type.JAMI

                ColumnLayout {
                    id: jamiRegisteredNameRectColumnLayout

                    spacing: 0

                    Text {
                        id: jamiRegisteredNameText

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: welcomePageColumnLayout.width
                        Layout.preferredHeight: 30

                        font.pointSize: JamiTheme.textFontSize + 1
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: textMetricsjamiRegisteredNameText.elidedText
                        color: JamiTheme.textColor
                        TextMetrics {
                            id: textMetricsjamiRegisteredNameText
                            font: jamiRegisteredNameText.font
                            text: UtilsAdapter.getBestId(LRCInstance.currentAccountId)
                            elideWidth: welcomePageColumnLayout.width
                            elide: Qt.ElideMiddle
                        }
                    }

                    PushButton {
                        id: copyRegisterednameButton

                        Layout.alignment: Qt.AlignCenter

                        preferredSize: 34
                        imagePadding: 4
                        imageColor: JamiTheme.textColor

                        source: JamiResources.content_copy_24dp_svg

                        onClicked: {
                            UtilsAdapter.setClipboardText(
                                        textMetricsjamiRegisteredNameText.text)
                        }
                    }
                }
            }

        }

        MaterialButton {
            id: btnAboutPopUp

            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.bottomMargin: JamiTheme.preferredMarginSize

            preferredWidth: JamiTheme.aboutButtonPreferredWidth

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            text: JamiStrings.aboutJami

            onClicked: aboutPopUpDialog.open()
        }
    }

    CustomBorder {
        commonBorder: false
        lBorderwidth: 1
        rBorderwidth: 0
        tBorderwidth: 0
        bBorderwidth: 0
        borderColor: JamiTheme.tabbarBorderColor
    }
}
