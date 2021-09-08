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

import net.jami.Adapters 1.1
import net.jami.Constants 1.1

BaseDialog {
    id: root

    // TODO: make MaterialButton ButtonStyle
    enum ButtonStyle {
        TintedBlue,
        TintedBlack,
        TintedRed
    }

    property var buttonTitles: []
    property var buttonCallBacks: []
    property var buttonStyles: []
    property alias infoText: infoText.text
    property alias innerContentData: innerContent.data

    function openWithParameters(title, info = "") {
        root.title = title
        if (info !== "")
            root.infoText = info
        open()
    }

    contentItem: Rectangle {
        id: container

        implicitWidth: Math.max(JamiTheme.preferredDialogWidth,
                                buttonTitles.length * (JamiTheme.preferredFieldWidth / 2
                                + JamiTheme.preferredMarginSize))
        implicitHeight: JamiTheme.preferredDialogHeight / 2 - JamiTheme.preferredMarginSize

        color: JamiTheme.secondaryBackgroundColor

        ColumnLayout {
            anchors.fill: parent

            Label {
                id: infoText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: JamiTheme.preferredDialogWidth - JamiTheme.preferredMarginSize
                Layout.topMargin: JamiTheme.preferredMarginSize

                font.pointSize: JamiTheme.menuFontSize - 2
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: JamiTheme.textColor
            }

            Item {
                id: innerContent
                Layout.topMargin: JamiTheme.preferredMarginSize / 2
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            RowLayout {
                spacing: JamiTheme.preferredMarginSize

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.bottomMargin: JamiTheme.preferredMarginSize

                Repeater {
                    model: buttonTitles.length
                    MaterialButton {
                        Layout.alignment: Qt.AlignVCenter

                        preferredWidth: JamiTheme.preferredFieldWidth / 2
                        preferredHeight: JamiTheme.preferredFieldHeight

                        color: {
                            switch(buttonStyles[modelData]) {
                            case SimpleMessageDialog.ButtonStyle.TintedBlue:
                                return JamiTheme.buttonTintedBlue
                            case SimpleMessageDialog.ButtonStyle.TintedBlack:
                                return JamiTheme.buttonTintedBlack
                            case SimpleMessageDialog.ButtonStyle.TintedRed:
                                return JamiTheme.buttonTintedRed
                            }
                        }
                        hoveredColor: {
                            switch(buttonStyles[modelData]) {
                            case SimpleMessageDialog.ButtonStyle.TintedBlue:
                                return JamiTheme.buttonTintedBlueHovered
                            case SimpleMessageDialog.ButtonStyle.TintedBlack:
                                return JamiTheme.buttonTintedBlackHovered
                            case SimpleMessageDialog.ButtonStyle.TintedRed:
                                return JamiTheme.buttonTintedRedHovered
                            }
                        }
                        pressedColor: {
                            switch(buttonStyles[modelData]) {
                            case SimpleMessageDialog.ButtonStyle.TintedBlue:
                                return JamiTheme.buttonTintedBluePressed
                            case SimpleMessageDialog.ButtonStyle.TintedBlack:
                                return JamiTheme.buttonTintedBlackPressed
                            case SimpleMessageDialog.ButtonStyle.TintedRed:
                                return JamiTheme.buttonTintedRedPressed
                            }
                        }
                        outlined: true

                        text: buttonTitles[modelData]

                        onClicked: {
                            if (buttonCallBacks[modelData])
                                buttonCallBacks[modelData]()
                            close()
                        }
                    }
                }
            }
        }
    }
}
