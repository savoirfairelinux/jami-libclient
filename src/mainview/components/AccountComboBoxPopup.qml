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
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Popup {
    id: root

    property bool toggleUpdatePopupHeight: false

    y: accountComboBox.height - 1
    implicitWidth: accountComboBox.width - 1

    // Hack - limite the accounts that can be shown.
    implicitHeight: {
        root.visible
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
        delegate: Rectangle {
            id: delegate

            width: root.width
            height: accountComboBox.height

            color: JamiTheme.backgroundColor

            AvatarImage {
                id: userImage

                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter

                width: 40
                height: 40

                presenceStatus: Status

                Component.onCompleted: {
                    return updateImage(
                                accountListModel.data(
                                    accountListModel.index(index, 0), AccountListModel.ID),
                                accountListModel.data(
                                    accountListModel.index(index, 0), AccountListModel.PictureUid))
                }
            }

            ColumnLayout {
                anchors.left: userImage.right
                anchors.leftMargin: 16
                anchors.top: delegate.top

                height: delegate.height

                spacing: 0

                Text {
                    id: textUserAliasPopup

                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.topMargin: textUsernamePopup.visible ?
                                          delegate.height / 2 - implicitHeight : 0

                    text: textMetricsUserAliasPopup.elidedText
                    font.pointSize: JamiTheme.textFontSize
                    color: JamiTheme.textColor

                    TextMetrics {
                        id: textMetricsUserAliasPopup
                        elide: Text.ElideRight
                        elideWidth: delegate.width - userImage.width - 80
                        text: Alias
                    }
                }

                Text {
                    id: textUsernamePopup

                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.bottomMargin: delegate.height / 2 - implicitHeight

                    visible: textMetricsUsernamePopup.text.length

                    text: textMetricsUsernamePopup.elidedText
                    font.pointSize: JamiTheme.textFontSize
                    color: JamiTheme.faddedLastInteractionFontColor

                    TextMetrics {
                        id: textMetricsUsernamePopup
                        elide: Text.ElideRight
                        elideWidth: delegate.width - userImage.width - 80
                        text: Username
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPressed: {
                    delegate.color = JamiTheme.pressColor
                }
                onReleased: {
                    delegate.color = JamiTheme.normalButtonColor
                    currentIndex = index
                    root.close()
                    AccountAdapter.accountChanged(index)
                }
                onEntered: {
                    delegate.color = JamiTheme.hoverColor
                }
                onExited: {
                    delegate.color = JamiTheme.backgroundColor
                }
            }
        }

        footer: Button {
            id: comboBoxFooterItem

            implicitWidth: accountComboBox.width
            implicitHeight: accountComboBox.height

            background: Rectangle {
                color: comboBoxFooterItem.hovered? JamiTheme.hoverColor : JamiTheme.backgroundColor
            }

            contentItem: Text {
                width: parent.width
                height: parent.height

                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter

                text: qsTr("Add Account") + "+"
                color: JamiTheme.textColor
            }
            font.pointSize: JamiTheme.textFontSize

            onClicked: {
                root.close()
                mainView.startWizard()
            }
        }

        ScrollIndicator.vertical: ScrollIndicator {}
    }

    background: Rectangle {
        id: accountComboBoxPopup

        color: JamiTheme.backgroundColor
        CustomBorder {
            commonBorder: false
            lBorderwidth: 2
            rBorderwidth: 1
            tBorderwidth: 1
            bBorderwidth: 2
            borderColor: JamiTheme.tabbarBorderColor
        }

        layer {
            enabled: true
            effect: DropShadow {
                color: JamiTheme.shadowColor
                verticalOffset: 2
                horizontalOffset: 2
                samples: 16
                radius: 10
            }
        }
    }
}
