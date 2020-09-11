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

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../constant"
import "../wizardview/components"

// Account Migration Dialog for migrating account

Window{
    id: accountMigrationDialog

    AccountsToMigrateListModel{
        id: accountsToMigrateListModel
    }

    property string accountID: ""
    property string password: ""

    property bool nonOperationClosing: true
    property bool successState : true
    property string imgBase64: ""

    signal accountMigrationFinished

    function startAccountMigrationOfTopStack(){
        passwordInputLineEdit.clear()
        accountsToMigrateListModel.reset()

        if(accountsToMigrateListModel.rowCount() <= 0){
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

        if(managerUri.length !== 0){
            managerUriInputLabel.text = managerUri
        } else {
            managerUriInputLabel.text = "N/A"
        }

        if(username.length !== 0){
            usernameInputLabel.text = username
        } else if(managerUsername.length !== 0){
            usernameInputLabel.text = managerUsername
        } else {
            usernameInputLabel.text = "N/A"
        }

        if(alias.length !== 0){
            aliasInputLabel.text = alias
        } else {
            aliasInputLabel.text = "N/A"
        }

        accountID = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.Account_ID)

        imgBase64 = accountsToMigrateListModel.data(accountsToMigrateListModel.index(
                                                        0, 0), AccountsToMigrateListModel.Picture)

        connectionMigrationEnded.enabled = false
        migrationPushButton.enabled = false
        stackedWidget.currentIndex = 0

        successState = true
        nonOperationClosing = true

        accountMigrationDialog.show()
        return true
    }

    function checkIfAccountMigrationFinishedAndClose(){
        accountsToMigrateListModel.reset()
        if(accountsToMigrateListModel.rowCount() > 0){
            startAccountMigrationOfTopStack()
        } else {
            accountMigrationFinished()
            if(!nonOperationClosing){
                nonOperationClosing = true
                accountMigrationDialog.close()
            }
        }
    }

    function acceptMigration(){
        nonOperationClosing = false
        accountsToMigrateListModel.dataChanged(accountsToMigrateListModel.index(0, 0),
                                               accountsToMigrateListModel.index(
                                               accountsToMigrateListModel.rowCount() - 1, 0))
        checkIfAccountMigrationFinishedAndClose()
    }

    function refuseMigrationAndDeleteAccount(){
        AccountAdapter.model.removeAccount(accountID)
        acceptMigration()
    }

    function closeWithoutOperation(){
        nonOperationClosing = false
        accountMigrationDialog.close()
    }

    Timer{
        id: timerFailureReturn

        interval: 1000
        repeat: false

        onTriggered: {
            stackedWidget.currentIndex = 0
            successState = true
        }
    }

    Connections{
        id: connectionMigrationEnded
        enabled: false
        target: AccountAdapter.model

        function onMigrationEnded(accountIdIn, ok){
            nonOperationClosing = true
            connectionMigrationEnded.enabled = false
            if(accountID !== accountIdIn){
                return
            }
            if(!ok){
                successState = false
                timerFailureReturn.restart()
            } else {
                acceptMigration()
            }
        }
    }

    function slotMigrationButtonClicked(){
        successState = true
        stackedWidget.currentIndex = 1

        connectionMigrationEnded.enabled = true
        AccountAdapter.setArchivePasswordAsync(accountID,password)
    }

    function slotDeleteButtonClicked(){
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

        if(nonOperationClosing){
            checkIfAccountMigrationFinishedAndClose()
        }
        nonOperationClosing = true
    }

    visible: false

    width: 455
    height: 594

    Component.onCompleted: {
        setX(Screen.width / 2 - width / 2)
        setY(Screen.height / 2 - height / 2)
    }

    ColumnLayout {
            anchors.fill: parent
            Layout.alignment: Qt.AlignHCenter

        StackLayout{
            id: stackedWidget

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            currentIndex: 0

            Rectangle{
                id: accountMigrationPage

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.topMargin: 11
                Layout.bottomMargin: 11

                x: (parent.width - width) /2
                y: (parent.height - height) /2

                ScrollView {
                    id: accountMigrationPageScroll
                    anchors.fill: parent

                    clip: true

                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                ColumnLayout {
                    spacing: 7

                    width: stackedWidget.width
                    height: stackedWidget.height
                    Layout.alignment: Qt.AlignHCenter

                    RowLayout {
                        Layout.fillWidth: true

                        Item{
                            Layout.fillWidth: true

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label {
                            id: accountMigrationLabel

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredHeight: 27

                            font.pointSize: 13
                            font.kerning: true
                            wrapMode:Text.Wrap

                            text: JamiStrings.authenticationRequired

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }
                    }

                    RowLayout{
                        spacing: 7

                        Item{
                            Layout.fillWidth: true

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: migrationReasonLabel

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredHeight: 21

                            font.pointSize: 10
                            font.kerning: true
                            wrapMode:Text.Wrap

                            text: JamiStrings.migrationReason

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }
                    }

                    Item{
                        Layout.minimumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.maximumWidth: 20

                        Layout.minimumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.maximumHeight: 20
                    }

                    ColumnLayout{
                        spacing: 7

                        Layout.fillWidth: true

                        RowLayout{
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignCenter

                            Item{
                                Layout.fillWidth: true

                                Layout.minimumHeight: 20
                                Layout.preferredHeight: 20
                                Layout.maximumHeight: 20
                            }

                            Label{
                                id: avatarLabel

                                Layout.maximumWidth: 200
                                Layout.preferredWidth: 200
                                Layout.minimumWidth: 200

                                Layout.maximumHeight: 200
                                Layout.preferredHeight: 200
                                Layout.minimumHeight: 200

                                Layout.alignment: Qt.AlignHCenter

                                background: Rectangle {
                                    id: avatarLabelBackground

                                    anchors.fill: parent
                                    color: "transparent"

                                    Image{
                                        id: avatarImg

                                        anchors.fill: parent
                                        source: {
                                            if(imgBase64.length === 0){
                                                return ""
                                            } else {
                                                return "data:image/png;base64," + imgBase64
                                            }
                                        }
                                        fillMode: Image.PreserveAspectCrop
                                        layer.enabled: true
                                        layer.effect: OpacityMask {
                                            maskSource: Rectangle{
                                                width: avatarImg.width
                                                height: avatarImg.height
                                                radius: {
                                                    var size = ((avatarImg.width <= avatarImg.height)? avatarImg.width:avatarImg.height)
                                                    return size /2
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            Item{
                                Layout.fillWidth: true

                                Layout.minimumHeight: 20
                                Layout.preferredHeight: 20
                                Layout.maximumHeight: 20
                            }
                        }
                    }

                    Item{
                        Layout.minimumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.maximumWidth: 20

                        Layout.minimumHeight: 30
                        Layout.preferredHeight: 30
                        Layout.maximumHeight: 30
                    }

                    GridLayout{
                        rows: 4
                        columns: 5
                        rowSpacing: 7
                        columnSpacing: 7

                        Layout.fillWidth: true

                        // 1st Row
                        Item{
                            Layout.row: 0
                            Layout.column: 0

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: aliasLabel

                            Layout.row: 0
                            Layout.column: 1

                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 30

                            text: JamiStrings.alias
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 0
                            Layout.column: 2

                            Layout.fillWidth: true
                            Layout.preferredWidth:40
                            Layout.maximumWidth: 40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: aliasInputLabel

                            Layout.row: 0
                            Layout.column: 3

                            Layout.preferredWidth: 142

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30

                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 0
                            Layout.column: 4

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        // 2nd Row
                        Item{
                            Layout.row: 1
                            Layout.column: 0

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: usernameLabel

                            Layout.row: 1
                            Layout.column: 1

                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 30

                            text: JamiStrings.username
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 1
                            Layout.column: 2

                            Layout.fillWidth: true
                            Layout.preferredWidth:40
                            Layout.maximumWidth: 40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: usernameInputLabel

                            Layout.row: 1
                            Layout.column: 3

                            Layout.preferredWidth: 142

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30

                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 1
                            Layout.column: 4

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        // 3rd Row
                        Item{
                            Layout.row: 2
                            Layout.column: 0

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: managerUriLabel

                            Layout.row: 2
                            Layout.column: 1

                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 30

                            text: JamiStrings.jamsServer
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 2
                            Layout.column: 2

                            Layout.fillWidth: true
                            Layout.preferredWidth:40
                            Layout.maximumWidth: 40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: managerUriInputLabel

                            Layout.row: 2
                            Layout.column: 3

                            Layout.preferredWidth: 142

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30

                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 2
                            Layout.column: 4

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        // 4th Row
                        Item{
                            Layout.row: 3
                            Layout.column: 0

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        Label{
                            id: passwordLabel

                            Layout.row: 3
                            Layout.column: 1

                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 30

                            text: JamiStrings.password
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.row: 3
                            Layout.column: 2

                            Layout.fillWidth: true
                            Layout.preferredWidth:40
                            Layout.maximumWidth: 40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }

                        InfoLineEdit{
                            id: passwordInputLineEdit

                            Layout.row: 3
                            Layout.column: 3

                            Layout.preferredWidth: 142

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30

                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            echoMode: TextInput.Password

                            placeholderText: JamiStrings.password

                            onTextChanged: {
                                if(text.length === 0){
                                    migrationPushButton.enabled = false
                                } else {
                                    migrationPushButton.enabled = true
                                }
                                password = text
                            }

                            onEditingFinished: {
                                password = text
                            }
                        }

                        Item{
                            Layout.row: 3
                            Layout.column: 4

                            Layout.fillWidth: true
                            Layout.preferredWidth:40

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }
                    }

                    Item{
                        Layout.minimumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.maximumWidth: 20

                        Layout.minimumHeight: 40
                        Layout.preferredHeight: 40
                        Layout.maximumHeight: 40
                    }

                    RowLayout{
                        spacing: 0
                        Layout.fillWidth: true

                        Item {
                            Layout.minimumWidth: 20
                            Layout.preferredWidth: 20
                            Layout.maximumWidth: 20

                            Layout.minimumHeight: 40
                            Layout.preferredHeight: 40
                            Layout.maximumHeight: 40
                        }

                        HoverableGradientButton {
                            id: migrationPushButton

                            Layout.alignment: Qt.AlignLeft
                            Layout.maximumWidth: 100
                            Layout.preferredWidth: 100

                            Layout.maximumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.minimumHeight: 30

                            text: JamiStrings.authenticate
                            font.pointSize: 10
                            font.kerning: true

                            radius: height / 2
                            backgroundColor: JamiTheme.releaseColor

                            onClicked: {
                                slotMigrationButtonClicked()
                            }
                        }

                        Item {
                            Layout.fillWidth: true

                            Layout.minimumHeight: 40
                            Layout.preferredHeight: 40
                            Layout.maximumHeight: 40
                        }

                        HoverableButtonTextItem {
                            id: deleteAccountPushButton

                            backgroundColor: "red"
                            onEnterColor: Qt.rgba(150 / 256, 0, 0, 0.7)
                            onDisabledBackgroundColor: Qt.rgba(
                                                           255 / 256,
                                                           0, 0, 0.8)
                            onPressColor: backgroundColor
                            textColor: "white"

                            Layout.alignment: Qt.AlignRight
                            Layout.maximumWidth: 100
                            Layout.preferredWidth: 100

                            Layout.maximumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.minimumHeight: 30

                            text: JamiStrings.deleteAccount
                            font.pointSize: 10
                            font.kerning: true

                            radius: height / 2

                            onClicked: {
                                slotDeleteButtonClicked()
                            }
                        }

                        Item {
                            Layout.minimumWidth: 20
                            Layout.preferredWidth: 20
                            Layout.maximumWidth: 20

                            Layout.minimumHeight: 40
                            Layout.preferredHeight: 40
                            Layout.maximumHeight: 40
                        }
                    }
                }

                }
            }

            Rectangle{
                id: migrationWaitingPage

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.topMargin: 11
                Layout.bottomMargin: 11

                x: (parent.width - width) /2
                y: (parent.height - height) /2

                ScrollView {
                    id: migrationWaitingPageScroll
                    anchors.fill: parent

                    clip: true

                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    ColumnLayout{
                        spacing: 7

                        width: stackedWidget.width
                        height: stackedWidget.height
                        Layout.alignment: Qt.AlignHCenter

                        Item{
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Layout.preferredHeight: 211
                        }

                        RowLayout{
                            spacing: 7

                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: true

                            Item{
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                Layout.minimumWidth: 20
                            }

                            Label{
                                id: spinnerLabel

                                Layout.alignment: Qt.AlignHCenter

                                Layout.maximumWidth: 200
                                Layout.preferredWidth: 200
                                Layout.minimumWidth: 200

                                Layout.maximumHeight: 200
                                Layout.preferredHeight: 200
                                Layout.minimumHeight: 200

                                property string spinnerDisplyState: successState ? "spinnerLabel_Regular" : "spinnerLabel_Failure"
                                onSpinnerDisplyStateChanged: {
                                    switch (spinnerDisplyState) {
                                    case "spinnerLabel_Regular":
                                        background = Qt.createQmlObject("import QtQuick 2.14;
                                                                            AnimatedImage {
                                                                            source: \"qrc:/images/jami_eclipse_spinner.gif\"

                                                                            playing: true
                                                                            paused: false
                                                                            fillMode: Image.PreserveAspectFit
                                                                            mipmap: true
                                                                            }", spinnerLabel)
                                        break
                                    case "spinnerLabel_Failure":
                                        background = Qt.createQmlObject("import QtQuick 2.14;
                                                                            import \"qrc:/src/constant/\";
                                                                            Image {
                                                                            anchors.fill: parent;
                                                                            source:\"image://tintedPixmap/\" + (\"qrc:/images/icons/baseline-error_outline-24px.svg\").replace(\"qrc:/images/icons/\", \"\") + \"+\" + JamiTheme.urgentOrange_;
                                                                            mipmap: true;}", spinnerLabel)
                                        break
                                    }
                                }
                            }

                            Item {
                                Layout.alignment: Qt.AlignRight
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                Layout.minimumWidth: 20
                            }
                        }

                        Item{
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Layout.preferredHeight: 211
                        }

                        Label{
                            id: progressLabel

                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: true

                            color: successState? "black" : "red"
                            text: successState? JamiStrings.inProgress : JamiStrings.authenticationFailed
                            font.pointSize: 11
                            font.kerning: true

                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Label.WordWrap
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.minimumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.maximumHeight: 20
                        }
                    }
                }
            }
        }
    }
}
