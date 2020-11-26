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

import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Enums 1.0
import net.jami.Constants 1.0

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

    Universal.theme: Universal.Light

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
        setX(Screen.width / 2 - width / 2)
        setY(Screen.height / 2 - height / 2)

        if (UtilsAdapter.getAccountListSize() !== 0) {
            mainApplicationLoader.setSource(JamiQmlUtils.mainViewLoadPath,
                                            {"containerWindow": root})
        } else {
            mainApplicationLoader.setSource(JamiQmlUtils.wizardViewLoadPath)
        }
    }

    function close(force = false) {
        // If we're in the onboarding wizard or 'MinimizeOnClose'
        // is set, then we can quit
        if (force || !SettingsAdapter.getAppValue(Settings.MinimizeOnClose) ||
                !UtilsAdapter.getAccountListSize()) {
            Qt.quit()
        } else
            hide()
    }

    AccountMigrationDialog{
        id: accountMigrationDialog

        visible: false

        onAccountMigrationFinished: startClient()
    }


    Loader {
        id: mainApplicationLoader

        anchors.fill: parent

        asynchronous: true
        visible: status == Loader.Ready
        source: ""

        Connections {
            target: mainApplicationLoader.item

            function onLoaderSourceChangeRequested(sourceToLoad) {
                if (sourceToLoad === MainApplicationWindow.LoadedSource.WizardView)
                    mainApplicationLoader.setSource(JamiQmlUtils.wizardViewLoadPath)
                else
                    mainApplicationLoader.setSource(JamiQmlUtils.mainViewLoadPath,
                                                    {"containerWindow": root})
            }
        }
    }

    overlay.modal: ColorOverlay {
        source: root.contentItem
        color: "transparent"

        // Color animation for overlay when pop up is shown.
        ColorAnimation on color {
            to: Qt.rgba(0, 0, 0, 0.33)
            duration: 500
        }
    }

    Connections {
        target: LRCInstance

        function onRestoreAppRequested() {
            requestActivate()
            showNormal()
        }

        function onNotificationClicked() {
            requestActivate()
            raise()
            if (visibility === Window.Hidden ||
                    visibility === Window.Minimized)
                showNormal()
        }
    }

    onClosing: root.close()

    onScreenChanged: JamiQmlUtils.mainApplicationScreen = root.screen

    Component.onCompleted: {
        if(!startAccountMigration()){
            startClient()
        }
        JamiQmlUtils.mainApplicationScreen = root.screen
    }
}
