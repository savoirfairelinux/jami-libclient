/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
import net.jami.Adapters 1.0

import "../../commoncomponents"

ComboBox {
    id: root

    signal settingBtnClicked

    // Reset accountListModel.
    function resetAccountListModel(accountId) {
        accountListModel.updateAvatarUid(accountId)
        accountListModel.reset()
    }

    Connections {
        target: accountListModel

        function onModelReset() {
            userImageRoot.updateImage(
                        AccountAdapter.currentAccountId,
                        accountListModel.data(
                            accountListModel.index(0, 0), AccountListModel.PictureUid))
            userImageRoot.presenceStatus =
                    accountListModel.data(accountListModel.index(0, 0), AccountListModel.Status)
            textMetricsUserAliasRoot.text = accountListModel.data(accountListModel.index(0,0),
                                                                  AccountListModel.Alias)
            textMetricsUsernameRoot.text = accountListModel.data(accountListModel.index(0,0),
                                                                 AccountListModel.Username)
        }
    }

    AvatarImage {
        id: userImageRoot

        anchors.left: root.left
        anchors.leftMargin: 16
        anchors.verticalCenter: root.verticalCenter

        width: 40
        height: 40

        imageId: AccountAdapter.currentAccountId

        presenceStatus: accountListModel.data(accountListModel.index(0, 0),
                                              AccountListModel.Status)
    }

    ColumnLayout {
        anchors.left: userImageRoot.right
        anchors.leftMargin: 16
        anchors.top: background.top

        height: root.height

        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.topMargin: textUsernameRoot.visible ? root.height / 2 - implicitHeight : 0

            Text {
                id: textUserAliasRoot

                Layout.alignment: Qt.AlignLeft

                text: textMetricsUserAliasRoot.elidedText
                font.pointSize: JamiTheme.textFontSize

                TextMetrics {
                    id: textMetricsUserAliasRoot

                    font: textUserAliasRoot.font
                    elide: Text.ElideRight
                    elideWidth: root.width - userImageRoot.width - settingsButton.width
                                - arrowDropDown.width - qrCodeGenerateButton.width - 55

                    text: accountListModel.data(accountListModel.index(0,0), AccountListModel.Alias)
                }
            }

            ResponsiveImage {
                id: arrowDropDown

                Layout.alignment: Qt.AlignRight

                width: 24
                height: 24

                source: "qrc:/images/icons/round-arrow_drop_down-24px.svg"
            }
        }

        Text {
            id: textUsernameRoot

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.bottomMargin: root.height / 2 - implicitHeight

            visible: textMetricsUsernameRoot.text.length

            text: textMetricsUsernameRoot.elidedText
            font.pointSize: JamiTheme.textFontSize
            color: JamiTheme.faddedLastInteractionFontColor

            TextMetrics {
                id: textMetricsUsernameRoot

                font: textUsernameRoot.font
                elide: Text.ElideRight
                elideWidth: root.width - userImageRoot.width - settingsButton.width
                            - qrCodeGenerateButton.width - 55

                text: accountListModel.data(accountListModel.index(0,0),
                                            AccountListModel.Username)
            }
        }
    }

    background: Rectangle {
        id: background

        implicitWidth: root.width
        implicitHeight: root.height
        color: JamiTheme.backgroundColor

        // TODO: this can be removed when frameless window is implemented
        Rectangle {
            height: 1
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            color: JamiTheme.tabbarBorderColor
        }

        states: [
            State {
                name: "open"; when: comboBoxPopup.opened
                PropertyChanges {
                    target: background
                    color: Qt.lighter(JamiTheme.hoverColor, 1.0)
                }
            },
            State {
                name: "hovered"
                PropertyChanges {
                    target: background
                    color: Qt.lighter(JamiTheme.hoverColor, 1.05)
                }
            },
            State {
                name: "normal"
                PropertyChanges {
                    target: background
                    color: JamiTheme.backgroundColor
                }
            }
        ]

        transitions: [
            Transition {
                to: "hovered"; reversible: true
                ColorAnimation { duration: JamiTheme.fadeDuration }
            }
        ]
    }

    MouseArea {
        id: comboBoxRootMouseArea

        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (comboBoxPopup.opened) {
                root.popup.close()
            } else {
                root.popup.open()
            }
        }
        onEntered: background.state = "hovered"
        onExited: {
            if (!comboBoxPopup.opened)
                background.state = "normal"
        }
    }

    Row {
        spacing: 10

        anchors.right: root.right
        anchors.rightMargin: 10
        anchors.verticalCenter: root.verticalCenter

        PushButton {
            id: qrCodeGenerateButton

            width: visible ? preferredSize : 0
            height: visible ? preferredSize : 0

            visible: AccountAdapter.currentAccountType === Profile.Type.RING
            toolTipText: JamiStrings.displayQRCode

            source: "qrc:/images/icons/qr_code-24px.svg"

            onClicked: {
                if (visible)
                    qrDialog.open()
            }
        }

        PushButton {
            id: settingsButton

            source: !mainViewWindow.inSettingsView ?
                        "qrc:/images/icons/round-settings-24px.svg" :
                        "qrc:/images/icons/round-close-24px.svg"

            toolTipText: !mainViewWindow.inSettingsView ?
                             JamiStrings.openSettings :
                             JamiStrings.closeSettings

            onClicked: {
                settingBtnClicked()
                background.state = "normal"
            }
        }
    }

    indicator: null

    // Overwrite the combo box pop up to add footer (for add accounts).
    popup: AccountComboBoxPopup {
        id: comboBoxPopup

        Shortcut {
            sequence: "Ctrl+J"
            context: Qt.ApplicationShortcut
            onActivated: comboBoxPopup.visible ?
                comboBoxPopup.close() :
                comboBoxPopup.open()
        }
    }

    Shortcut {
        sequence: "Ctrl+,"
        context: Qt.ApplicationShortcut
        onActivated: settingBtnClicked()
    }
}
