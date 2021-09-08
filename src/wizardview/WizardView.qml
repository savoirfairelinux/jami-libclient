/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1
import net.jami.Enums 1.1

import "../"
import "../commoncomponents"
import "components"

Rectangle {
    id: root

    // signal to redirect the page to main view
    signal loaderSourceChangeRequested(int sourceToLoad)

    color: JamiTheme.backgroundColor

    Connections{
        target: AccountAdapter

        // reportFailure
        function onReportFailure() {
            var errorMessage = JamiStrings.errorCreateAccount

            for (var i = 0; i < controlPanelStackView.children.length; i++) {
                if (i === controlPanelStackView.currentIndex) {
                    controlPanelStackView.children[i].errorOccured(errorMessage)
                    return
                }
            }
        }
    }

    Connections {
        target: WizardViewStepModel

        function onCloseWizardView() {
            loaderSourceChangeRequested(MainApplicationWindow.LoadedSource.MainView)
        }
    }

    JamiFlickable {
        id: wizardViewScrollView

        property ScrollBar vScrollBar: ScrollBar.vertical

        anchors.fill: parent

        contentHeight: controlPanelStackView.height

        StackLayout {
            id: controlPanelStackView

            objectName: "controlPanelStackView"

            function setPage(obj) {
                for (var i in this.children) {
                    if (this.children[i] === obj) {
                        currentIndex = i
                        return
                    }
                }
            }

            anchors.centerIn: parent

            width: wizardViewScrollView.width

            WelcomePage {
                id: welcomePage

                objectName: "welcomePage"

                onShowThisPage: controlPanelStackView.setPage(this)

                onScrollToBottom: {
                    if (welcomePage.preferredHeight > root.height)
                        wizardViewScrollView.vScrollBar.position = 1
                }
            }

            CreateAccountPage {
                id: createAccountPage

                objectName: "createAccountPage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            ProfilePage {
                id: profilePage

                objectName: "profilePage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            BackupKeyPage {
                id: backupKeysPage

                objectName: "backupKeysPage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            ImportFromDevicePage {
                id: importFromDevicePage

                objectName: "importFromDevicePage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            ImportFromBackupPage {
                id: importFromBackupPage

                objectName: "importFromBackupPage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            ConnectToAccountManagerPage {
                id: connectToAccountManagerPage

                objectName: "connectToAccountManagerPage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            CreateSIPAccountPage {
                id: createSIPAccountPage

                objectName: "createSIPAccountPage"

                onShowThisPage: controlPanelStackView.setPage(this)
            }

            Component.onCompleted: {
                // avoid binding loop
                height = Qt.binding(function (){
                    var index = currentIndex
                            === WizardViewStepModel.MainSteps.CreateRendezVous ?
                                WizardViewStepModel.MainSteps.CreateJamiAccount : currentIndex
                    return Math.max(
                                controlPanelStackView.itemAt(index).preferredHeight,
                                wizardViewScrollView.height)
                })
            }
        }
    }
}
