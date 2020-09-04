
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

Popup {
    id: comboBoxPopup

    property bool toogleUpdatePopupHeight: false

    signal accountNeedToChange(int index)
    signal newAccountButtonClicked

    y: accountComboBox.height - 1
    implicitWidth: accountComboBox.width - 1


    // Hack - limite the accounts that can be shown.
    implicitHeight: {
        comboBoxPopup.visible
        return Math.min(accountComboBox.height *
                        Math.min(5, accountListModel.rowCount() + 1),
                        mainViewSidePanelRect.height)
    }
    padding: 0

    contentItem: ListView {
        id: comboBoxPopupListView


        // In list view, index is an interger.
        clip: true
        model: accountListModel
        implicitHeight: contentHeight
        delegate: ItemDelegate {
            Image {
                id: userImage

                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter

                width: 30
                height: 30

                fillMode: Image.PreserveAspectFit
                mipmap: true

                // Role::Picture
                source: {
                    var data = accountListModel.data(accountListModel.index(index, 0), 259)
                    if (data === undefined) {
                        return ""
                    }
                    return "data:image/png;base64," + data
                }

                AccountPresenceCycle {
                    id: accountPresenseCycle

                    anchors.right: userImage.right
                    anchors.rightMargin: -2
                    anchors.bottom: userImage.bottom
                    anchors.bottomMargin: -2

                    accountStatus: Status
                }
            }

            Text {
                id: textUserAliasPopup

                anchors.left: userImage.right
                anchors.leftMargin: 10
                anchors.top: itemCoboBackground.top
                anchors.topMargin: 15

                text: textMetricsUserAliasPopup.elidedText
                font.pointSize: JamiTheme.textFontSize
            }

            Text {
                id: textUsernamePopup

                anchors.left: userImage.right
                anchors.leftMargin: 10
                anchors.top: textUserAliasPopup.bottom

                text: textMetricsUsernamePopup.elidedText
                font.pointSize: JamiTheme.textFontSize
                color: JamiTheme.faddedLastInteractionFontColor
            }

            TextMetrics {
                id: textMetricsUserAliasPopup
                elide: Text.ElideRight
                elideWidth: accountComboBox.width - userImage.width - settingsButton.width - 30
                text: Alias
            }

            TextMetrics {
                id: textMetricsUsernamePopup
                elide: Text.ElideRight
                elideWidth: accountComboBox.width - userImage.width - settingsButton.width - 30
                text: Username
            }

            background: Rectangle {
                id: itemCoboBackground
                color: JamiTheme.backgroundColor
                implicitWidth: accountComboBox.width
                implicitHeight: accountComboBox.height
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPressed: {
                    itemCoboBackground.color = JamiTheme.pressColor
                }
                onReleased: {
                    itemCoboBackground.color = JamiTheme.releaseColor
                    currentIndex = index
                    comboBoxPopup.close()
                    comboBoxPopup.accountNeedToChange(index)
                }
                onEntered: {
                    itemCoboBackground.color = JamiTheme.hoverColor
                }
                onExited: {
                    itemCoboBackground.color = JamiTheme.backgroundColor
                }
            }
        }

        footer: Button {
            id: comboBoxFooterItem

            implicitWidth: accountComboBox.width
            implicitHeight: accountComboBox.height

            background: Rectangle {
                color: comboBoxFooterItem.hovered? JamiTheme.releaseColor : JamiTheme.backgroundColor
            }

            text: qsTr("Add Account") + "+"

            onClicked: {
                comboBoxPopup.close()
                comboBoxPopup.newAccountButtonClicked()
            }
        }

        ScrollIndicator.vertical: ScrollIndicator {}
    }
    background: Rectangle {
        id: accountComboBoxPopup
        color: JamiTheme.backgroundColor
        CustomBorder {
            commonBorder: false
            lBorderwidth: 1
            rBorderwidth: 1
            tBorderwidth: 1
            bBorderwidth: 1
            borderColor: JamiTheme.tabbarBorderColor
        }
    }
}
