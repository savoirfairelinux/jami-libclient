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

BaseModalDialog {
    id: root

    property int preferredHeight: 0

    width: 400

    onPopupContentLoadStatusChanged: {
        if (popupContentLoadStatus === Loader.Ready)
            preferredHeight = Qt.binding(function() {
                return popupContentLoader.item.contentHeight
            })
    }

    popupContent: JamiFlickable {
        id: aboutPopUpScrollView

        contentHeight: aboutPopUpContentRectColumnLayout.implicitHeight

        ColumnLayout {
            id: aboutPopUpContentRectColumnLayout

            width: Math.max(root.width, implicitWidth)
            height: Math.max(aboutPopUpScrollView.height, implicitHeight)

            ResponsiveImage {
                id: aboutPopUPJamiLogoImage

                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: JamiTheme.preferredMarginSize
                Layout.preferredWidth: 250
                Layout.preferredHeight: 88

                source: JamiTheme.darkTheme ?
                            JamiResources.logo_jami_standard_coul_white_svg :
                            JamiResources.logo_jami_standard_coul_svg
            }

            MaterialLineEdit {
                id: jamiVersionText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpScrollView.width

                font.pointSize: JamiTheme.textFontSize

                padding: 0
                readOnly: true
                selectByMouse: true

                text: JamiStrings.version + ": " + UtilsAdapter.getVersionStr()
                color: JamiTheme.textColor

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Label {
                id: jamiSlogansText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpScrollView.width
                Layout.preferredHeight: textMetricsjamiSlogansText.boundingRect.height
                Layout.topMargin: 5

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiSlogansText.text
                color: JamiTheme.textColor

                TextMetrics {
                    id: textMetricsjamiSlogansText
                    font: jamiSlogansText.font
                    text: JamiStrings.slogan
                }
            }

            Label {
                id: jamiDeclarationText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpScrollView.width
                Layout.preferredHeight: 40
                Layout.topMargin: 5

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize
                color: JamiTheme.textColor

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                // TextMetrics does not work for multi-line.
                text: JamiStrings.declaration
            }

            Label {
                id: jamiDeclarationHyperText

                Layout.alignment: Qt.AlignCenter

                // Strangely, hoveredLink works badly when width grows too large
                Layout.preferredWidth: 50
                Layout.preferredHeight: textMetricsjamiDeclarationHyperText.boundingRect.height
                Layout.topMargin: 5
                Layout.bottomMargin: 5
                color: JamiTheme.textColor

                font.pointSize: JamiTheme.textFontSize
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

                    // We don't want to eat clicks on the Text.
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Label {
                id: jamiDeclarationYearText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpScrollView.width
                Layout.preferredHeight: textMetricsjamiDeclarationYearText.boundingRect.height
                Layout.bottomMargin: 5

                font.pointSize: JamiTheme.textFontSize
                color: JamiTheme.textColor

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: textMetricsjamiDeclarationYearText.text

                TextMetrics {
                    id: textMetricsjamiDeclarationYearText
                    font: jamiDeclarationYearText.font
                    text: JamiStrings.companyDeclarationYear
                }
            }

            Label {
                id: jamiNoneWarrantyHyperText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 300
                Layout.preferredHeight: textMetricsjamiNoneWarrantyHyperText.boundingRect.height * 2
                Layout.bottomMargin: 10

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.tinyFontSize

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: JamiTheme.textColor

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

            ProjectCreditsScrollView {
                id: projectCreditsScrollView

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: aboutPopUpScrollView.width - JamiTheme.preferredMarginSize * 2
                Layout.preferredHeight: 128
                Layout.margins: JamiTheme.preferredMarginSize
            }

            MaterialButton {
                id: btnClose

                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: JamiTheme.preferredMarginSize

                preferredWidth: JamiTheme.preferredFieldWidth / 2
                preferredHeight: JamiTheme.preferredFieldHeight

                text: JamiStrings.close
                color: enabled ? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                hoveredColor: JamiTheme.buttonTintedBlackHovered
                pressedColor: JamiTheme.buttonTintedBlackPressed
                outlined: true

                onClicked: close()
            }
        }
    }
}
