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
    function resetAccountListModel() {
        accountListModel.reset()
    }

    Connections {
        target: accountListModel

        function onModelReset() {
            userImageRoot.source = "data:image/png;base64," + accountListModel.data(
                        accountListModel.index(0, 0), AccountListModel.Picture)
            currentAccountPresenseCycle.accountStatus =
                    accountListModel.data(accountListModel.index(0, 0), AccountListModel.Status)
            textMetricsUserAliasRoot.text = accountListModel.data(accountListModel.index(0,0),
                                                                  AccountListModel.Alias)
            textMetricsUsernameRoot.text = accountListModel.data(accountListModel.index(0,0),
                                                                 AccountListModel.Username)
        }
    }

    Image {
        id: userImageRoot

        anchors.left: root.left
        anchors.leftMargin: 16
        anchors.verticalCenter: root.verticalCenter

        width: 30
        height: 30

        fillMode: Image.PreserveAspectFit

        // Base 64 format
        source: "data:image/png;base64," + accountListModel.data(
                            accountListModel.index(0, 0), AccountListModel.Picture)
        mipmap: true

        AccountPresenceCycle {
            id: currentAccountPresenseCycle

            anchors.right: userImageRoot.right
            anchors.rightMargin: -2
            anchors.bottom: userImageRoot.bottom
            anchors.bottomMargin: -2

            accountStatus: accountListModel.data(accountListModel.index(0, 0),
                                                 AccountListModel.Status)
        }
    }

    Text {
        id: textUserAliasRoot

        anchors.left: userImageRoot.right
        anchors.leftMargin: 16
        anchors.top: rootItemBackground.top
        anchors.topMargin: 16

        text: textMetricsUserAliasRoot.elidedText
        font.pointSize: JamiTheme.textFontSize
    }

    ResponsiveImage {
        id: arrowDropDown

        anchors.left: textUserAliasRoot.right
        anchors.verticalCenter: textUserAliasRoot.verticalCenter

        width: 24
        height: 24

        source: "qrc:/images/icons/round-arrow_drop_down-24px.svg"
    }

    Text {
        id: textUsernameRoot

        anchors.left: userImageRoot.right
        anchors.leftMargin: 16
        anchors.top: textUserAliasRoot.bottom

        text: textMetricsUsernameRoot.elidedText
        font.pointSize: JamiTheme.textFontSize
        color: JamiTheme.faddedLastInteractionFontColor
    }

    TextMetrics {
        id: textMetricsUserAliasRoot

        font: textUserAliasRoot.font
        elide: Text.ElideRight
        elideWidth: root.width - userImageRoot.width - settingsButton.width
                    - arrowDropDown.width - qrCodeGenerateButton.width - 55

        text: accountListModel.data(accountListModel.index(0,0), AccountListModel.Alias)
    }

    TextMetrics {
        id: textMetricsUsernameRoot

        font: textUsernameRoot.font
        elide: Text.ElideRight
        elideWidth: root.width - userImageRoot.width - settingsButton.width
                    - qrCodeGenerateButton.width - 55

        text: accountListModel.data(accountListModel.index(0,0),
                                    AccountListModel.Username)
    }

    background: Rectangle {
        id: rootItemBackground

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
        onEntered: rootItemBackground.color = Qt.lighter(JamiTheme.hoverColor, 1.05)
        onExited: rootItemBackground.color = JamiTheme.backgroundColor
    }

    PushButton {
        id: qrCodeGenerateButton

        anchors.right: settingsButton.left
        anchors.rightMargin: 10
        anchors.verticalCenter: root.verticalCenter

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

        anchors.right: root.right
        anchors.rightMargin: 10
        anchors.verticalCenter: root.verticalCenter

        source: !mainViewWindow.inSettingsView ?
                    "qrc:/images/icons/round-settings-24px.svg" :
                    "qrc:/images/icons/round-close-24px.svg"

        toolTipText: !mainViewWindow.inSettingsView ?
                         JamiStrings.openSettings :
                         JamiStrings.closeSettings

        onClicked: {
            settingBtnClicked()
            rootItemBackground.color = JamiTheme.backgroundColor
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
