
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
    id: aboutPopUp


    /*
     * When dialog is opened, trigger mainViewWindow overlay which is defined in overlay.model (model : true is necessary).
     */
    modal: true


    /*
     * Content height + margin.
     */
    contentHeight: aboutPopUpContentRectColumnLayout.height + 5 * 7

    ProjectCreditsScrollView {
        id: projectCreditsScrollView

        visible: false
    }

    ChangeLogScrollView {
        id: changeLogScrollView

        visible: false
    }

    Rectangle {
        id: aboutPopUpContentRect

        anchors.fill: parent

        ColumnLayout {
            id: aboutPopUpContentRectColumnLayout

            Image {
                id: aboutPopUPJamiLogoImage

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: 100

                fillMode: Image.PreserveAspectFit
                source: "qrc:/images/logo-jami-standard-coul.png"
                mipmap: true
            }

            Label {
                id: jamiVersionText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: textMetricsjamiVersionText.boundingRect.height

                font.pointSize: JamiTheme.textFontSize - 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiVersionText.text

                TextMetrics {
                    id: textMetricsjamiVersionText
                    font: jamiVersionText.font
                    text: qsTr("version") + ": " + ClientWrapper.utilsAdaptor.getVersionStr()
                }
            }

            Label {
                id: jamiSlogansText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: textMetricsjamiSlogansText.boundingRect.height
                Layout.topMargin: 5

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize - 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiSlogansText.text

                TextMetrics {
                    id: textMetricsjamiSlogansText
                    font: jamiSlogansText.font
                    text: qsTr("Together")
                }
            }

            Label {
                id: jamiDeclarationText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: 40
                Layout.topMargin: 5

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize - 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter


                /*
                 * TextMetrics does not work for multi-line.
                 */
                text: qsTr("The Qt client for Jami.\nJami is a secured and distributed communication software.")
            }

            Label {
                id: jamiDeclarationHyperText

                Layout.alignment: Qt.AlignCenter


                /*
                 * Strangely, hoveredLink works badly when width grows too large
                 */
                Layout.preferredWidth: 50
                Layout.preferredHeight: textMetricsjamiDeclarationHyperText.boundingRect.height
                Layout.topMargin: 5
                Layout.bottomMargin: 5

                font.pointSize: JamiTheme.textFontSize - 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiDeclarationHyperText.text
                onLinkActivated: Qt.openUrlExternally(link)

                TextMetrics {
                    id: textMetricsjamiDeclarationHyperText
                    font: jamiDeclarationHyperText.font
                    text: '<html><style type="text/css"></style><a href="https://jami.net">jami.net</a></html>'
                }

                MouseArea {
                    anchors.fill: parent


                    /*
                     * We don't want to eat clicks on the Text.
                     */
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Label {
                id: jamiDeclarationYearText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: textMetricsjamiDeclarationYearText.boundingRect.height
                Layout.bottomMargin: 5

                font.pointSize: JamiTheme.textFontSize - 2

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiDeclarationYearText.text

                TextMetrics {
                    id: textMetricsjamiDeclarationYearText
                    font: jamiDeclarationYearText.font
                    text: "© 2015-2020 Savoir-faire Linux"
                }
            }

            Label {
                id: jamiNoneWarrantyHyperText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: Math.min(300,
                                                aboutPopUpContentRect.width)
                Layout.preferredHeight: textMetricsjamiNoneWarrantyHyperText.boundingRect.height * 2
                Layout.bottomMargin: 10

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize - 3

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiNoneWarrantyHyperText.text
                onLinkActivated: Qt.openUrlExternally(link)

                TextMetrics {
                    id: textMetricsjamiNoneWarrantyHyperText
                    font: jamiDeclarationHyperText.font
                    text: '<html><style type="text/css"></style>This program comes with absolutely no warranty.<br\>See the <a href="http://www.gnu.org/licenses/gpl-3.0.html">GNU General Public License, version 3 or later</a> for details.</html>'
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Rectangle {
                id: buttonGroupChangeLogAndCredits

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: 30
                Layout.bottomMargin: 10

                RowLayout {
                    id: buttonGroupChangeLogAndCreditsRowLayout

                    anchors.fill: parent

                    MaterialButton {
                        id: changeLogButton
                        text: qsTr("Changelog")
                        color: projectCreditsScrollView.visible? JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedGrey
                        Layout.preferredWidth: 100

                        onClicked: {
                            if (changeLogOrCreditsStack.depth > 1) {
                                changeLogOrCreditsStack.pop()
                            }
                        }
                    }

                    MaterialButton {
                        id: creditsButton
                        text: qsTr("Credit")
                        color: projectCreditsScrollView.visible? JamiTheme.buttonTintedGrey : JamiTheme.buttonTintedGreyInactive
                        Layout.preferredWidth: 100

                        onClicked: {
                            if (changeLogOrCreditsStack.depth == 1) {
                                changeLogOrCreditsStack.push(
                                            projectCreditsScrollView)
                            }
                        }
                    }
                }
            }

            StackView {
                id: changeLogOrCreditsStack

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpContentRect.width
                Layout.preferredHeight: 150
                Layout.bottomMargin: 5

                initialItem: changeLogScrollView

                clip: true
            }
        }
    }

    background: Rectangle {
        border.width: 0
        radius: 10
    }
}
