/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Helpers 1.1
import net.jami.Constants 1.1

import "mainview"
import "wizardview"
import "commoncomponents"

ApplicationWindow {
    id: root

    enum LoadedSource {
        WizardView = 0,
        MainView,
        None
    }

    property ApplicationWindow appWindow : root
    property bool isFullScreen: false

    function toggleFullScreen() {
        isFullScreen = !isFullScreen
    }

    function checkLoadedSource() {
        var sourceString = mainApplicationLoader.source.toString()

        if (sourceString === JamiQmlUtils.wizardViewLoadPath)
            return MainApplicationWindow.LoadedSource.WizardView
        else if (sourceString === JamiQmlUtils.mainViewLoadPath)
            return MainApplicationWindow.LoadedSource.MainView

        return MainApplicationWindow.LoadedSource.None
    }

    function startAccountMigration(){
        return accountMigrationDialog.startAccountMigrationOfTopStack()
    }

    function startClient(){
        if (UtilsAdapter.getAccountListSize() !== 0) {
            mainApplicationLoader.setSource(JamiQmlUtils.mainViewLoadPath)
        } else {
            mainApplicationLoader.setSource(JamiQmlUtils.wizardViewLoadPath)
        }
    }

    function close(force = false) {
        // If we're in the onboarding wizard or 'MinimizeOnClose'
        // is set, then we can quit
        if (force || !UtilsAdapter.getAppValue(Settings.MinimizeOnClose) ||
                !UtilsAdapter.getAccountListSize()) {
            Qt.quit()
        } else
            hide()
    }

    visibility: !visible ?
                   Window.Hidden : (isFullScreen ?
                                        Window.FullScreen :
                                        Window.Windowed)

    title: JamiStrings.appTitle

    width: {
        if (checkLoadedSource() === MainApplicationWindow.LoadedSource.WizardView)
            return JamiTheme.wizardViewMinWidth
        return JamiTheme.mainViewPreferredWidth
    }
    height: {
        if (checkLoadedSource() === MainApplicationWindow.LoadedSource.WizardView)
            return JamiTheme.wizardViewMinHeight
        return JamiTheme.mainViewPreferredHeight
    }
    minimumWidth: {
        if (checkLoadedSource() === MainApplicationWindow.LoadedSource.WizardView)
            return JamiTheme.wizardViewMinWidth
        return JamiTheme.mainViewMinWidth
    }
    minimumHeight: {
        if (checkLoadedSource() === MainApplicationWindow.LoadedSource.WizardView)
            return JamiTheme.wizardViewMinHeight
        return JamiTheme.mainViewMinHeight
    }

    visible: mainApplicationLoader.status === Loader.Ready

    // To facilitate reparenting of the callview during
    // fullscreen mode, we need QQuickItem based object.
    Item {
        id: appContainer

        anchors.fill: parent
    }

    AccountMigrationDialog {
        id: accountMigrationDialog

        visible: false

        onAccountMigrationFinished: startClient()
    }

    DaemonReconnectPopup {
        id: daemonReconnectPopup
    }

    Loader {
        id: mainApplicationLoader

        anchors.fill: parent
        z: -1

        asynchronous: true
        visible: status == Loader.Ready
        source: ""

        Connections {
            target: mainApplicationLoader.item

            function onLoaderSourceChangeRequested(sourceToLoad) {
                if (sourceToLoad === MainApplicationWindow.LoadedSource.WizardView)
                    mainApplicationLoader.setSource(JamiQmlUtils.wizardViewLoadPath)
                else
                    mainApplicationLoader.setSource(JamiQmlUtils.mainViewLoadPath)
            }
        }

        onLoaded: {
            // Quiet check for updates on start if set to.
            if (UtilsAdapter.getAppValue(Settings.AutoUpdate)) {
                UpdateManager.checkForUpdates(true)
                UpdateManager.setAutoUpdateCheck(true)
            }
        }
    }

    Connections {
        target: LRCInstance

        function onRestoreAppRequested() {
            requestActivate()
            if (isFullScreen)
                showFullScreen()
            else
                showNormal()
        }

        function onNotificationClicked() {
            requestActivate()
            raise()
            if (visibility === Window.Hidden ||
                    visibility === Window.Minimized) {
                if (isFullScreen)
                    showFullScreen()
                else
                    showNormal()
            }
        }
    }

    Connections {
        target: {
            if (Qt.platform.os !== "windows")
                return DBusErrorHandler
            return null
        }
        ignoreUnknownSignals: true

        function onShowDaemonReconnectPopup(visible) {
            if (visible)
                daemonReconnectPopup.open()
            else
                daemonReconnectPopup.close()
        }

        function onDaemonReconnectFailed() {
            daemonReconnectPopup.connectionFailed = true
        }
    }

    Overlay.modal: ColorOverlay {
        source: root.contentItem
        color: "transparent"

        // Color animation for overlay when pop up is shown.
        ColorAnimation on color {
            to: Qt.rgba(0, 0, 0, 0.33)
            duration: 500
        }
    }

    onClosing: root.close()

    onScreenChanged: JamiQmlUtils.mainApplicationScreen = root.screen

    Component.onCompleted: {
        if(!startAccountMigration()){
            startClient()
        }
        JamiQmlUtils.mainApplicationScreen = root.screen

        if (Qt.platform.os !== "windows")
            DBusErrorHandler.setActive(true)
    }
}
