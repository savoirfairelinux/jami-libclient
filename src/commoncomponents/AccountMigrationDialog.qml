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

import "../wizardview/components"

// Account Migration Dialog for migrating account

Window {
    id: accountMigrationDialog

    AccountsToMigrateListModel {
        id: accountsToMigrateListModel

        lrcInstance: LRCInstance
    }

    property string accountID: ""
    property string password: ""

    property bool nonOperationClosing: true
    property bool successState : true

    signal accountMigrationFinished

    function startAccountMigrationOfTopStack() {
        passwordInputLineEdit.clear()
        accountsToMigrateListModel.reset()

        if (accountsToMigrateListModel.rowCount() <= 0) {
            closeWithoutOperation()

            return false
        }

        var managerUsername = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.ManagerUsername)
        var managerUri = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.ManagerUri)
        var username = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.Username)
        var alias = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.Alias)

        if (managerUri.length !== 0) {
            managerUriInputLabel.text = managerUri
        } else {
            managerUriInputLabel.text = "N/A"
        }

        if (username.length !== 0) {
            usernameInputLabel.text = username
        } else if (managerUsername.length !== 0) {
            usernameInputLabel.text = managerUsername
        } else {
            usernameInputLabel.text = "N/A"
        }

        if (alias.length !== 0) {
            aliasInputLabel.text = alias
        } else {
            aliasInputLabel.text = "N/A"
        }

        accountID = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.Account_ID)

        connectionMigrationEnded.enabled = false
        migrationPushButton.enabled = false
        stackedWidget.currentIndex = 0

        successState = true
        nonOperationClosing = true

        accountMigrationDialog.show()
        return true
    }

    function checkIfAccountMigrationFinishedAndClose() {
        accountsToMigrateListModel.reset()
        if (accountsToMigrateListModel.rowCount() > 0) {
            startAccountMigrationOfTopStack()
        } else {
            accountMigrationFinished()
            if (!nonOperationClosing) {
                nonOperationClosing = true
                accountMigrationDialog.close()
            }
        }
    }

    function acceptMigration() {
        nonOperationClosing = false
        accountsToMigrateListModel.dataChanged(accountsToMigrateListModel.index(0, 0),
                                               accountsToMigrateListModel.index(
                                               accountsToMigrateListModel.rowCount() - 1, 0))
        checkIfAccountMigrationFinishedAndClose()
    }

    function refuseMigrationAndDeleteAccount() {
        AccountAdapter.model.removeAccount(accountID)
        acceptMigration()
    }

    function closeWithoutOperation() {
        nonOperationClosing = false
        accountMigrationDialog.close()
    }

    Timer {
        id: timerFailureReturn

        interval: 1000
        repeat: false

        onTriggered: {
            stackedWidget.currentIndex = 0
            successState = true
        }
    }

    Connections {
        id: connectionMigrationEnded
        enabled: false
        target: AccountAdapter.model

        function onMigrationEnded(accountIdIn, ok) {
            nonOperationClosing = true
            connectionMigrationEnded.enabled = false
            if (accountID !== accountIdIn) {
                return
            }
            if (ok) {
                acceptMigration()
            } else {
                successState = false
                timerFailureReturn.restart()
            }
        }
    }

    function slotMigrationButtonClicked() {
        successState = true
        stackedWidget.currentIndex = 1

        connectionMigrationEnded.enabled = true
        AccountAdapter.setArchivePasswordAsync(accountID,password)
    }

    function slotDeleteButtonClicked() {
        nonOperationClosing = false
        refuseMigrationAndDeleteAccount()
    }

    onClosing: {
        connectionMigrationEnded.enabled = false
        stackedWidget.currentIndex = 0
        accountID = ""
        password = ""
        passwordInputLineEdit.clear()
        managerUriInputLabel.text = ""
        usernameInputLabel.text = ""
        aliasInputLabel.text = ""

        if (nonOperationClosing) {
            checkIfAccountMigrationFinishedAndClose()
        }
        nonOperationClosing = true
    }

    visible: false

    title: JamiStrings.authenticate
    flags: Qt.WindowStaysOnTopHint

    width: 600
    height: 600
    minimumWidth: 600
    minimumHeight: 600

    Component.onCompleted: {
        setX(Screen.width / 2 - width / 2)
        setY(Screen.height / 2 - height / 2)
    }

    ColumnLayout {
        anchors.fill: parent
        Layout.alignment: Qt.AlignHCenter

        StackLayout {
            id: stackedWidget

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            currentIndex: 0

            // Index = 0
            Rectangle {
                id: accountMigrationPage

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter

                ColumnLayout {
                    spacing: 8

                    width: stackedWidget.width
                    height: stackedWidget.height
                    Layout.alignment: Qt.AlignHCenter

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

                    Label {
                        id: avatarLabel

                        Layout.preferredWidth: 200
                        Layout.preferredHeight: 200

                        Layout.alignment: Qt.AlignHCenter

                        background: Rectangle {
                            id: avatarLabelBackground

                            anchors.fill: parent
                            color: "transparent"

                            Avatar {
                                anchors.fill: parent
                                showPresenceIndicator: false
                                mode: Avatar.Mode.Account
                                imageId: accountID
                            }
                        }
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

                            onTextChanged: {
                                migrationPushButton.enabled = text.length > 0
                                password = text
                            }

                            onEditingFinished: {
                                password = text
                            }
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

                            text: JamiStrings.authenticate

                            onClicked: {
                                slotMigrationButtonClicked()
                            }
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
                            onClicked: {
                                slotDeleteButtonClicked()
                            }
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

                        Label {
                            id: spinnerLabel

                            Layout.alignment: Qt.AlignHCenter

                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 200

                            property string spinnerDisplyState: successState ? "spinnerLabel_Regular" : "spinnerLabel_Failure"
                            onSpinnerDisplyStateChanged: {
                                switch (spinnerDisplyState) {
                                case "spinnerLabel_Regular":
                                    background = Qt.createQmlObject("import QtQuick;
                                                                        import \"qrc:/src/constant/\";
                                                                        AnimatedImage {
                                                                        source: JamiResources.jami_eclipse_spinner_gif
                                                                        playing: true
                                                                        paused: false
                                                                        fillMode: Image.PreserveAspectFit
                                                                        mipmap: true}", spinnerLabel)
                                    break
                                case "spinnerLabel_Failure":
                                    background = Qt.createQmlObject("import QtQuick;
                                                                        import \"qrc:/src/constant/\";
                                                                        Image {
                                                                        anchors.fill: parent;
                                                                        source: JamiResources.error_outline_black_24dp_svg;
                                                                        mipmap: true;}", spinnerLabel)
                                    break
                                }
                            }
                        }
                    }

                    Label {
                        id: progressLabel

                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: true
                        Layout.bottomMargin: 80

                        color: successState? "black" : "red"
                        text: successState? JamiStrings.inProgress : JamiStrings.authenticationFailed
                        font.pointSize: JamiTheme.textFontSize
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
