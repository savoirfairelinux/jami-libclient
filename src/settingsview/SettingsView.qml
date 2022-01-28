/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "components"
import "../mainview/js/contactpickercreation.js" as ContactPickerCreation


Rectangle {
    id: root

    enum SettingsMenu {
        Account,
        General,
        Media,
        Plugin
    }

    onVisibleChanged: {
        if(visible){
            setSelected(selectedMenu,true)
        } else {
            AccountAdapter.stopPreviewing()
        }
    }

    function setSelected(sel, recovery = false) {
        profileType = SettingsAdapter.getCurrentAccount_Profile_Info_Type()

        if(selectedMenu === sel && (!recovery)) { return }
        switch(sel) {
            case SettingsView.Account:
                pageIdCurrentAccountSettings.connectCurrentAccount()
                AccountAdapter.stopPreviewing()
                selectedMenu = sel
                pageIdCurrentAccountSettings.updateAccountInfoDisplayed()
                break
            case SettingsView.General:
                AccountAdapter.stopPreviewing()
                selectedMenu = sel
                break
            case SettingsView.Media:
                selectedMenu = sel
                avSettings.populateAVSettings()
                break
            case SettingsView.Plugin:
                AccountAdapter.stopPreviewing()
                selectedMenu = sel
                pluginSettings.populatePluginSettings()
                break
        }
    }

    Connections {
        id: accountListChangedConnection
        target: LRCInstance

        function onAccountListChanged() {
            accountListChanged()
        }
    }

    // slots
    function leaveSettingsSlot(showMainView) {
        AccountAdapter.stopPreviewing()
        settingsViewRect.stopBooth()
        if (showMainView)
            settingsViewNeedToShowMainView()
        else
            settingsViewNeedToShowNewWizardWindow()
    }

    function accountListChanged() {
        var accountList = AccountAdapter.model.getAccountList()
        if(accountList.length === 0)
            return
        pageIdCurrentAccountSettings.disconnectAccountConnections()
        var device = AVModel.getDefaultDevice()
        if(device.length === 0) {
            AVModel.setCurrentVideoCaptureDevice(device)
        }
    }

    property int profileType: SettingsAdapter.getCurrentAccount_Profile_Info_Type()
    property int selectedMenu: SettingsView.Account
    // signal to redirect the page to main view
    signal settingsViewNeedToShowMainView()
    signal settingsViewNeedToShowNewWizardWindow

    signal settingsBackArrowClicked

    visible: true

    Rectangle {
        id: settingsViewRect

        anchors.fill: root
        color: JamiTheme.secondaryBackgroundColor

        signal stopBooth

        property bool isSIP: {
            switch (profileType) {
                case Profile.Type.SIP:
                    return true;
                default:
                    return false;
            }
        }

        SettingsHeader {
            id: settingsHeader

            anchors.top: settingsViewRect.top
            anchors.left: settingsViewRect.left
            anchors.leftMargin: {
                var pageWidth = rightSettingsStackLayout.itemAt(
                            rightSettingsStackLayout.currentIndex).contentWidth
                return (settingsViewRect.width - pageWidth) / 2 + JamiTheme.preferredMarginSize
            }

            height: JamiTheme.settingsHeaderpreferredHeight

            title: {
                switch(selectedMenu){
                    case SettingsView.Account:
                        return qsTr("Account Settings")
                    case SettingsView.General:
                        return qsTr("General")
                    case SettingsView.Media:
                        return JamiStrings.avSettingsTitle
                    case SettingsView.Plugin:
                        return qsTr("Plugin")
                }
            }

            onBackArrowClicked: root.settingsBackArrowClicked()
        }

        Flickable {
            id: settingsViewScrollView

            anchors.top: settingsHeader.bottom
            anchors.horizontalCenter: settingsViewRect.horizontalCenter

            height: settingsViewRect.height - settingsHeader.height
            width: settingsViewRect.width

            maximumFlickVelocity: 1024
            ScrollBar.vertical: ScrollBar { }

            clip: true
            contentHeight: rightSettingsStackLayout.height

            StackLayout {
                id: rightSettingsStackLayout

                anchors.centerIn: parent

                width: settingsViewScrollView.width

                property int pageIdCurrentAccountSettingsPage: 0
                property int pageIdGeneralSettingsPage: 1
                property int pageIdAvSettingPage: 2
                property int pageIdPluginSettingsPage: 3

                currentIndex: {
                    switch(selectedMenu){
                        case SettingsView.Account:
                            return pageIdCurrentAccountSettingsPage
                        case SettingsView.General:
                            return pageIdGeneralSettingsPage
                        case SettingsView.Media:
                            return pageIdAvSettingPage
                        case SettingsView.Plugin:
                            return pageIdPluginSettingsPage
                    }
                }

                Component.onCompleted: {
                    // avoid binding loop
                    height = Qt.binding(function (){
                        return Math.max(
                                    rightSettingsStackLayout.itemAt(currentIndex).preferredHeight,
                                    settingsViewScrollView.height)
                    })
                }

                // current account setting scroll page, index 0
                CurrentAccountSettings {
                    id: pageIdCurrentAccountSettings

                    Layout.alignment: Qt.AlignCenter

                    isSIP: settingsViewRect.isSIP

                    onNavigateToMainView: {
                        leaveSettingsSlot(true)
                    }

                    onNavigateToNewWizardView: {
                        leaveSettingsSlot(false)
                    }

                    onAdvancedSettingsToggled: {
                        if (settingsVisible)
                            settingsViewScrollView.contentY = getAdvancedSettingsScrollPosition()
                        else
                            settingsViewScrollView.contentY = 0
                    }
                }

                // general setting page, index 1
                GeneralSettingsPage {
                    id: generalSettings

                    Layout.alignment: Qt.AlignCenter
                }

                // av setting page, index 2
                AvSettingPage {
                    id: avSettings

                    Layout.alignment: Qt.AlignCenter
                }

                // plugin setting page, index 3
                PluginSettingsPage {
                    id: pluginSettings

                    Layout.alignment: Qt.AlignCenter
                }
            }
        }
    }
}
