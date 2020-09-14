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
import net.jami.Adapters 1.0

import "../../commoncomponents"

ComboBox {
    id: root

    signal accountChanged(int index)
    signal needToBackToWelcomePage
    signal newAccountButtonClicked
    signal settingBtnClicked

    // Reset accountListModel.
    function resetAccountListModel() {
        accountListModel.reset()
    }

    Connections {
        target: accountListModel

        function onModelReset() {
            userImageRoot.source = "data:image/png;base64," + accountListModel.data(
                        accountListModel.index(0, 0), 259)
            currentAccountPresenseCycle.accountStatus =
                    accountListModel.data(accountListModel.index(0, 0), 261)
            textMetricsUserAliasRoot.text = accountListModel.data(accountListModel.index(0,0), 257)
            textMetricsUsernameRoot.text = accountListModel.data(accountListModel.index(0,0), 258)
        }
    }

    Connections {
        target: AccountAdapter

        function onNavigateToWelcomePageRequested() {
            needToBackToWelcomePage()
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
                            accountListModel.index(0, 0), 259)
        mipmap: true

        AccountPresenceCycle {
            id: currentAccountPresenseCycle

            anchors.right: userImageRoot.right
            anchors.rightMargin: -2
            anchors.bottom: userImageRoot.bottom
            anchors.bottomMargin: -2

            accountStatus: accountListModel.data(accountListModel.index(0, 0), 261)
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

    Image {
        id: arrowDropDown

        anchors.left: textUserAliasRoot.right
        anchors.verticalCenter: textUserAliasRoot.verticalCenter

        width: 24
        height: 24

        fillMode: Image.PreserveAspectFit
        mipmap: true
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

        // Role::Alias
        text: accountListModel.data(accountListModel.index(0,0), 257)
    }

    TextMetrics {
        id: textMetricsUsernameRoot

        font: textUsernameRoot.font
        elide: Text.ElideRight
        elideWidth: root.width - userImageRoot.width - settingsButton.width
                    - qrCodeGenerateButton.width - 55



        // Role::Username
        text: accountListModel.data(accountListModel.index(0,0), 258)
    }

    HoverableButton {
        id: qrCodeGenerateButton

        anchors.right: settingsButton.left
        anchors.rightMargin: 10
        anchors.verticalCenter: root.verticalCenter

        buttonImageHeight: height - 8
        buttonImageWidth: width - 8
        radius: height / 2

        width: 24
        height: 24

        visible: AccountAdapter.currentAccountType === Profile.Type.RING
        toolTipText: JamiStrings.displayQRCode
        hoverEnabled: true

        source: "qrc:/images/qrcode.png"
        backgroundColor: "white"
        onClicked: {
            if (visible)
                qrDialog.open()
        }
    }

    HoverableButton {
        id: settingsButton

        anchors.right: root.right
        anchors.rightMargin: 10
        anchors.verticalCenter: root.verticalCenter

        buttonImageHeight: height - 8
        buttonImageWidth: width - 8
        radius: height / 2
        width: 25
        height: 25

        source: !mainViewWindow.inSettingsView ?
                    "qrc:/images/icons/round-settings-24px.svg" :
                    "qrc:/images/icons/round-close-24px.svg"
        toolTipText: !mainViewWindow.inSettingsView ?
                         JamiStrings.openSettings :
                         JamiStrings.closeSettings
        hoverEnabled: true

        backgroundColor: "white"
        onClicked: {
            settingBtnClicked()
        }
    }

    background: Rectangle {
        id: rootItemBackground

        implicitWidth: root.width
        implicitHeight: root.height
        color: JamiTheme.backgroundColor
    }

    MouseArea {
        id: comboBoxRootMouseArea

        anchors.fill: parent

        hoverEnabled: true
        propagateComposedEvents: true

        onPressed: {
            if (isMouseOnButton(mouse, qrCodeGenerateButton)) {
                qrCodeGenerateButton.backgroundColor = JamiTheme.pressColor
                qrCodeGenerateButton.clicked()
            }if (isMouseOnButton(mouse, settingsButton)) {
                settingsButton.backgroundColor = JamiTheme.pressColor
                settingsButton.clicked()
            } else {
                rootItemBackground.color = JamiTheme.pressColor
            }
        }

        onReleased: {
            if (isMouseOnButton(mouse, qrCodeGenerateButton)) {
                qrCodeGenerateButton.backgroundColor = JamiTheme.releaseColor
            } else if (isMouseOnButton(mouse, settingsButton)) {
                settingsButton.backgroundColor = JamiTheme.releaseColor
            } else {
                rootItemBackground.color = JamiTheme.releaseColor
                if (comboBoxPopup.opened) {
                    root.popup.close()
                } else {
                    root.popup.open()
                }
            }
        }
        onEntered: {
            rootItemBackground.color = JamiTheme.hoverColor
        }
        onExited: {
            rootItemBackground.color = JamiTheme.backgroundColor
        }
        onMouseXChanged: {

            // Manually making button hover.
            qrCodeGenerateButton.backgroundColor = (isMouseOnButton(mouse, qrCodeGenerateButton)) ?
                        JamiTheme.hoverColor : "white"

            settingsButton.backgroundColor = (isMouseOnButton(mouse, settingsButton)) ?
                        JamiTheme.hoverColor : "white"
        }

        function isMouseOnButton(mouse, button) {
            var mousePos = mapToItem(comboBoxRootMouseArea, mouse.x, mouse.y)
            var qrButtonPos = mapToItem(comboBoxRootMouseArea,
                                              button.x,
                                              button.y)
            if ((mousePos.x >= qrButtonPos.x
                 && mousePos.x <= qrButtonPos.x + button.width)
                    && (mousePos.y >= qrButtonPos.y
                        && mousePos.y <= qrButtonPos.y + button.height))
                return true
            return false
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

        onAccountNeedToChange: {
            root.accountChanged(index)
        }

        onNewAccountButtonClicked: {
            root.newAccountButtonClicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+,"
        context: Qt.ApplicationShortcut
        onActivated: settingBtnClicked()
    }
}
