/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "commoncomponents"

// Account Migration Dialog for migrating account
Rectangle {
    id: root

    enum AccountMigrationStep {
        PasswordEnter,
        Synching
    }

    property bool successState: true

    // signal to redirect the page to main view
    signal loaderSourceChangeRequested(int sourceToLoad)

    function slotMigrationButtonClicked() {
        stackedWidget.currentIndex = AccountMigrationView.AccountMigrationStep.Synching

        AccountAdapter.setArchivePasswordAsync(
                    CurrentAccountToMigrate.accountId, passwordInputLineEdit.text)
    }

    function slotDeleteButtonClicked() {
        stackedWidget.currentIndex = AccountMigrationView.AccountMigrationStep.Synching

        CurrentAccountToMigrate.removeCurrentAccountToMigrate()
    }

    color: JamiTheme.backgroundColor

    Timer {
        id: timerFailureReturn

        interval: 1000
        repeat: false

        onTriggered: {
            stackedWidget.currentIndex =
                    AccountMigrationView.AccountMigrationStep.PasswordEnter
            successState = true
        }
    }

    Connections {
        id: connectionMigrationEnded

        target: CurrentAccountToMigrate

        function onMigrationEnded(ok) {
            successState = ok

            if (ok) {
                passwordInputLineEdit.clear()

                stackedWidget.currentIndex =
                        AccountMigrationView.AccountMigrationStep.PasswordEnter
            } else {
                timerFailureReturn.restart()
            }
        }

        function onCurrentAccountToMigrateRemoved() {
            successState = true
            passwordInputLineEdit.clear()

            stackedWidget.currentIndex =
                    AccountMigrationView.AccountMigrationStep.PasswordEnter
        }

        function onAllMigrationsFinished() {
            if (UtilsAdapter.getAccountListSize() === 0)
                root.loaderSourceChangeRequested(
                            MainApplicationWindow.LoadedSource.WizardView)
            else
                root.loaderSourceChangeRequested(
                            MainApplicationWindow.LoadedSource.MainView)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        Layout.alignment: Qt.AlignHCenter

        StackLayout {
            id: stackedWidget

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            // Index = 0
            Rectangle {
                id: accountMigrationPage

                ColumnLayout {
                    spacing: 8

                    anchors.fill: accountMigrationPage

                    Label {
                        id: accountMigrationLabel

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true
                        wrapMode:Text.Wrap

                        text: JamiStrings.authenticationRequired

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Label {
                        id: migrationReasonLabel

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true
                        wrapMode:Text.Wrap

                        text: JamiStrings.migrationReason

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Avatar {
                        id: avatarLabel

                        Layout.preferredWidth: 200
                        Layout.preferredHeight: 200

                        Layout.alignment: Qt.AlignHCenter

                        showPresenceIndicator: false
                        mode: Avatar.Mode.Account
                        imageId: CurrentAccountToMigrate.accountId
                    }

                    GridLayout {
                        rows: 4
                        columns: 2
                        rowSpacing: 8
                        columnSpacing: 8

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: JamiTheme.preferredMarginSize
                        Layout.rightMargin: JamiTheme.preferredMarginSize

                        // 1st Row
                        Label {
                            id: aliasLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: JamiStrings.alias
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Label {
                            id: aliasInputLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: {
                                if (CurrentAccountToMigrate.alias.length !== 0) {
                                    return CurrentAccountToMigrate.alias
                                } else {
                                    return JamiStrings.notAvailable
                                }
                            }
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        // 2nd Row
                        Label {
                            id: usernameLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: JamiStrings.username
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Label {
                            id: usernameInputLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: {
                                if (CurrentAccountToMigrate.username.length !== 0) {
                                    return CurrentAccountToMigrate.username
                                } else if (CurrentAccountToMigrate.managerUsername.length !== 0) {
                                    return CurrentAccountToMigrate.managerUsername
                                } else {
                                    return JamiStrings.notAvailable
                                }
                            }
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        // 3rd Row
                        Label {
                            id: managerUriLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: JamiStrings.jamsServer
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Label {
                            id: managerUriInputLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: {
                                if (CurrentAccountToMigrate.managerUri.length !== 0) {
                                    return CurrentAccountToMigrate.managerUri
                                } else {
                                    return JamiStrings.notAvailable
                                }
                            }
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        // 4th Row
                        Label {
                            id: passwordLabel

                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            text: JamiStrings.password
                            font.pointSize: JamiTheme.textFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        MaterialLineEdit {
                            id: passwordInputLineEdit

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredHeight: 48

                            echoMode: TextInput.Password
                            placeholderText: JamiStrings.password

                            onAccepted: slotMigrationButtonClicked()
                        }
                    }

                    RowLayout {
                        spacing: 80

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        Layout.bottomMargin: JamiTheme.preferredMarginSize

                        MaterialButton {
                            id: migrationPushButton

                            Layout.alignment: Qt.AlignLeft

                            preferredWidth: JamiTheme.preferredFieldWidth / 2

                            color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                            hoveredColor: JamiTheme.buttonTintedBlackHovered
                            pressedColor: JamiTheme.buttonTintedBlackPressed
                            outlined: true
                            enabled: passwordInputLineEdit.text.length > 0

                            text: JamiStrings.authenticate

                            onClicked: slotMigrationButtonClicked()
                        }

                        MaterialButton {
                            id: deleteAccountPushButton

                            Layout.alignment: Qt.AlignRight

                            preferredWidth: JamiTheme.preferredFieldWidth / 2

                            color: JamiTheme.buttonTintedRed
                            hoveredColor: JamiTheme.buttonTintedRedHovered
                            pressedColor: JamiTheme.buttonTintedRedPressed
                            outlined: true

                            text: JamiStrings.deleteAccount
                            onClicked: slotDeleteButtonClicked()
                        }
                    }
                }
            }

            // Index = 1
            Rectangle {
                id: migrationWaitingPage

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter

                ColumnLayout {
                    spacing: 8

                    width: stackedWidget.width
                    height: stackedWidget.height
                    Layout.alignment: Qt.AlignHCenter

                    RowLayout{
                        spacing: 8

                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: true

                        ResponsiveImage {
                            id: errorLabel

                            Layout.alignment: Qt.AlignHCenter

                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 200

                            containerHeight: Layout.preferredHeight
                            containerWidth: Layout.preferredWidth

                            visible: !successState

                            source: JamiResources.round_remove_circle_24dp_svg
                            color: JamiTheme.redColor
                        }

                        AnimatedImage {
                            id: spinnerLabel

                            Layout.alignment: Qt.AlignHCenter

                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 200

                            visible: successState

                            source: JamiResources.jami_eclipse_spinner_gif

                            playing: successState
                            fillMode: Image.PreserveAspectFit
                            mipmap: true
                        }
                    }

                    Label {
                        id: progressLabel

                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: true
                        Layout.bottomMargin: 80

                        color: successState ? JamiTheme.textColor : JamiTheme.redColor
                        text: successState ? JamiStrings.inProgress :
                                             JamiStrings.authenticationFailed
                        font.pointSize: JamiTheme.textFontSize + 5
                        font.kerning: true

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Label.WordWrap
                    }
                }
            }
        }
    }
}
