
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

ComboBox {
    id: accountComboBox

    signal accountChanged(int index)
    signal needToBackToWelcomePage(int index)
    signal needToUpdateSmartList(string accountId)
    signal newAccountButtonClicked
    signal settingBtnClicked

    currentIndex: 0

    function backToWelcomePage(index) {
        accountComboBox.needToBackToWelcomePage(index)
    }

    function updateSmartList(accountId) {
        accountComboBox.needToUpdateSmartList(accountId)
    }


    /*
     * Refresh every item in accountListModel.
     */
    function updateAccountListModel() {
        accountListModel.dataChanged(accountListModel.index(0, 0),
                                     accountListModel.index(
                                         accountListModel.rowCount() - 1, 0))
    }


    /*
     * Reset accountListModel.
     */
    function resetAccountListModel() {
        accountListModel.reset()
    }

    Image {
        id: userImageRoot

        anchors.left: accountComboBox.left
        anchors.leftMargin: 5
        anchors.verticalCenter: accountComboBox.verticalCenter

        width: accountComboBox.height - 10
        height: accountComboBox.height - 10

        fillMode: Image.PreserveAspectFit


        /*
         * Base 64 format
         */
        source: {
            if (currentIndex !== -1)
                return "data:image/png;base64," + accountListModel.data(
                            accountListModel.index(
                                accountComboBox.currentIndex, 0), 259)
            else
                return source
        }
        mipmap: true

        Rectangle {
            id: presenseRect

            anchors.right: userImageRoot.right
            anchors.rightMargin: 1
            anchors.bottom: userImageRoot.bottom
            anchors.bottomMargin: 2

            width: 14
            height: 14


            /*
             * Visible when account is registered, enum REGISTERED == 5.
             */
            visible: {
                if (currentIndex !== -1)
                    return accountListModel.data(
                                accountListModel.index(
                                    accountComboBox.currentIndex, 0), 261) === 5
                else
                    return visible
            }

            Rectangle {
                id: presenseCycle

                anchors.centerIn: presenseRect

                width: 10
                height: 10

                radius: 30
                color: JamiTheme.presenceGreen
            }

            radius: 30
            color: "white"
        }
    }

    Text {
        id: textUserAliasRoot

        anchors.left: userImageRoot.right
        anchors.leftMargin: 10
        anchors.top: rootItemBackground.top
        anchors.topMargin: 5

        text: textMetricsUserAliasRoot.elidedText
        font.pointSize: JamiTheme.textFontSize
    }

    Text {
        id: textUsernameRoot

        anchors.left: userImageRoot.right
        anchors.leftMargin: 10
        anchors.top: textUserAliasRoot.bottom
        anchors.topMargin: 5

        text: textMetricsUsernameRoot.elidedText
        font.pointSize: JamiTheme.textFontSize
        color: JamiTheme.faddedFontColor
    }

    TextMetrics {
        id: textMetricsUserAliasRoot

        font: textUserAliasRoot.font
        elide: Text.ElideMiddle
        elideWidth: accountComboBox.width - userImageRoot.width - settingsButton.width - 25


        /*
         * Role::Alias
         */
        text: {
            if (currentIndex !== -1)
                return accountListModel.data(accountListModel.index(
                                                 accountComboBox.currentIndex,
                                                 0), 257)
            else
                return text
        }
    }

    TextMetrics {
        id: textMetricsUsernameRoot

        font: textUsernameRoot.font
        elide: Text.ElideMiddle
        elideWidth: accountComboBox.width - userImageRoot.width - settingsButton.width - 25


        /*
         * Role::Username
         */
        text: {
            if (currentIndex !== -1)
                return accountListModel.data(accountListModel.index(
                                                 accountComboBox.currentIndex,
                                                 0), 258)
            else
                return text
        }
    }

    HoverableButton {
        id: settingsButton

        anchors.right: accountComboBox.right
        anchors.rightMargin: 10
        anchors.verticalCenter: accountComboBox.verticalCenter

        buttonImageHeight: height - 8
        buttonImageWidth: width - 8
        source: "qrc:/images/icons/round-settings-24px.svg"
        radius: height / 2
        width: 25
        height: 25

        onClicked: {
            settingBtnClicked()
        }
    }

    background: Rectangle {
        id: rootItemBackground

        implicitWidth: accountComboBox.width
        implicitHeight: accountComboBox.height

        border.width: 0
    }

    MouseArea {
        id: comboBoxRootMouseArea

        anchors.fill: parent

        hoverEnabled: true
        propagateComposedEvents: true

        onPressed: {
            if (isMouseOnSettingsButton(mouse)) {
                settingsButton.backgroundColor = JamiTheme.pressColor
                settingsButton.clicked()
            } else
                rootItemBackground.color = JamiTheme.pressColor
        }
        onReleased: {
            if (isMouseOnSettingsButton(mouse)) {
                settingsButton.backgroundColor = JamiTheme.releaseColor
            } else {
                rootItemBackground.color = JamiTheme.releaseColor
                if (comboBoxPopup.opened) {
                    accountComboBox.popup.close()
                } else {
                    accountComboBox.popup.open()
                }
            }
        }
        onEntered: {
            rootItemBackground.color = JamiTheme.hoverColor
        }
        onExited: {
            rootItemBackground.color = "white"
        }
        onMouseXChanged: {


            /*
             * Manually making settings button hover.
             */
            if (isMouseOnSettingsButton(mouse)) {
                settingsButton.backgroundColor = JamiTheme.hoverColor
                rootItemBackground.color = "white"
            } else {
                settingsButton.backgroundColor = "white"
                rootItemBackground.color = JamiTheme.hoverColor
            }
        }
        function isMouseOnSettingsButton(mouse) {
            var mousePos = mapToItem(comboBoxRootMouseArea, mouse.x, mouse.y)
            var settingsButtonPos = mapToItem(comboBoxRootMouseArea,
                                              settingsButton.x,
                                              settingsButton.y)
            if ((mousePos.x >= settingsButtonPos.x
                 && mousePos.x <= settingsButtonPos.x + settingsButton.width)
                    && (mousePos.y >= settingsButtonPos.y
                        && mousePos.y <= settingsButtonPos.y + settingsButton.height))
                return true
            return false
        }
    }

    indicator: null


    /*
     * Overwrite the combo box pop up to add footer (for add accounts).
     */
    popup: AccountComboBoxPopup {
        id: comboBoxPopup

        onAccountNeedToChange: {
            accountComboBox.accountChanged(index)
        }

        onNewAccountButtonClicked: {
            accountComboBox.newAccountButtonClicked()
        }
    }
}
