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

import QtQuick 2.15
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "components"

Rectangle {
    id: root

    enum SettingsMenu{
        Account,
        General,
        Media,
        Plugin
    }

    onVisibleChanged: {
        if(visible){
            setSelected(selectedMenu,true)
        }
    }

    function setSelected(sel, recovery = false) {
        profileType = SettingsAdapter.getCurrentAccount_Profile_Info_Type()

        if(selectedMenu === sel && (!recovery)) { return }
        switch(sel) {
            case SettingsView.Account:
                pageIdCurrentAccountSettings.connectCurrentAccount()

                settingsViewRect.stopAudioMeter()
                settingsViewRect.stopPreviewing()

                selectedMenu = sel

                if(pageIdCurrentAccountSettings.isPhotoBoothOpened())
                {
                    settingsViewRect.setAvatar()
                }

                pageIdCurrentAccountSettings.updateAccountInfoDisplayed()
                break
            case SettingsView.General:
                try{
                    settingsViewRect.stopAudioMeter()
                    settingsViewRect.stopPreviewing()
                } catch(erro) {}

                selectedMenu = sel
                break
            case SettingsView.Media:
                selectedMenu = sel

                settingsViewRect.stopPreviewing()
                avSettings.populateAVSettings()
                settingsViewRect.startAudioMeter()
                break
            case SettingsView.Plugin:
                try{
                    settingsViewRect.stopAudioMeter()
                    settingsViewRect.stopPreviewing()
                } catch(erro) {}

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
        settingsViewRect.stopAudioMeter()
        settingsViewRect.stopPreviewing()
        settingsViewRect.stopBooth()
        if (showMainView)
            settingsViewWindowNeedToShowMainViewWindow()
        else
            settingsViewWindowNeedToShowNewWizardWindow()
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
    signal settingsViewWindowNeedToShowMainViewWindow()
    signal settingsViewWindowNeedToShowNewWizardWindow

    signal settingsBackArrowClicked

    visible: true

    Rectangle {
        id: settingsViewRect
        anchors.fill: root

        signal stopAudioMeter
        signal startAudioMeter
        signal stopPreviewing
        signal stopBooth
        signal setAvatar

        property bool isSIP: {
            switch (profileType) {
                case Profile.Type.SIP:
                    return true;
                default:
                    return false;
            }
        }

        StackLayout {
            id: rightSettingsWidget

            anchors.fill: parent

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

            // current account setting scroll page, index 0
            CurrentAccountSettings {
                id: pageIdCurrentAccountSettings

                Layout.fillHeight: true
                Layout.fillWidth: true

                isSIP: settingsViewRect.isSIP

                onNavigateToMainView: {
                    leaveSettingsSlot(true)
                }

                onNavigateToNewWizardView: {
                    leaveSettingsSlot(false)
                }
            }

            // general setting page, index 1
            GeneralSettingsPage {
                id: generalSettings

                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            // av setting page, index 2
            AvSettingPage {
                id: avSettings

                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            // plugin setting page, index 3
            PluginSettingsPage {
                id: pluginSettings

                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }


    // Back button signal redirection
    Component.onCompleted: {
        pageIdCurrentAccountSettings.backArrowClicked.connect(settingsBackArrowClicked)
        generalSettings.backArrowClicked.connect(settingsBackArrowClicked)
        avSettings.backArrowClicked.connect(settingsBackArrowClicked)
        pluginSettings.backArrowClicked.connect(settingsBackArrowClicked)
    }
}
