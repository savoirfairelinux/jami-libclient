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
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.platform

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property int preferredHeight: importFromBackupPageColumnLayout.implicitHeight

    property string fileImportBtnText: JamiStrings.archive
    property string filePath: ""
    property string errorText: ""

    signal showThisPage

    function clearAllTextFields() {
        connectBtn.spinnerTriggered = false
        passwordFromBackupEdit.clear()
        filePath = ""
        errorText = ""
        fileImportBtnText = JamiStrings.archive
    }

    function errorOccured(errorMessage) {
        errorText = errorMessage
        connectBtn.spinnerTriggered = false
    }

    Connections {
        target: WizardViewStepModel

        function onMainStepChanged() {
            if (WizardViewStepModel.mainStep === WizardViewStepModel.MainSteps.AccountCreation &&
                    WizardViewStepModel.accountCreationOption ===
                    WizardViewStepModel.AccountCreationOption.ImportFromBackup) {
                clearAllTextFields()
                root.showThisPage()
            }
        }
    }

    color: JamiTheme.backgroundColor

    JamiFileDialog {
        id: importFromFileDialog

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.openFile
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [JamiStrings.jamiArchiveFiles + " (*.gz)", JamiStrings.allFiles + " (*)"]

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

        onRejected: {
            fileImportBtn.forceActiveFocus()
        }

        onAccepted: {
            filePath = file
            if (file.length !== "") {
                fileImportBtnText = UtilsAdapter.toFileInfoName(file)
            } else {
                fileImportBtnText = JamiStrings.archive
            }
        }
    }

    ColumnLayout {
        id: importFromBackupPageColumnLayout

        spacing: JamiTheme.wizardViewPageLayoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Text {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins

            text: JamiStrings.importFromBackup
            color: JamiTheme.textColor
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialButton {
            id: fileImportBtn

            objectName: "fileImportBtn"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            text: fileImportBtnText
            toolTipText: JamiStrings.importAccountArchive
            iconSource: JamiResources.round_folder_24dp_svg
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            KeyNavigation.tab: passwordFromBackupEdit
            KeyNavigation.up: {
                if (backButton.visible)
                    return backButton
                else if (connectBtn.enabled)
                    return connectBtn
                return passwordFromBackupEdit
            }
            KeyNavigation.down: KeyNavigation.tab

            onClicked: {
                errorText = ""
                importFromFileDialog.open()
            }
        }

        Text {
            // For multiline text, recursive rearrange warning will show up when
            // directly assigning contentHeight to Layout.preferredHeight
            property int preferredHeight: JamiTheme.wizardViewPageLayoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: fileImportBtn.width
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.importAccountExplanation
            color: JamiTheme.textColor
            wrapMode: Text.Wrap

            onTextChanged: function (text) {
                var boundingRect = JamiQmlUtils.getTextBoundingRect(font, text)
                preferredHeight += (boundingRect.width / fileImportBtn.preferredWidth)
                        * boundingRect.height
            }
        }

        MaterialLineEdit {
            id: passwordFromBackupEdit

            objectName: "passwordFromBackupEdit"

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            focus: visible

            selectByMouse: true
            placeholderText: JamiStrings.password
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true

            echoMode: TextInput.Password

            KeyNavigation.tab: {
                if (connectBtn.enabled)
                    return connectBtn
                else if (backButton.visible)
                    return backButton
                return fileImportBtn
            }
            KeyNavigation.up: fileImportBtn
            KeyNavigation.down: KeyNavigation.tab

            onTextChanged: errorText = ""
            onAccepted: {
                if (connectBtn.enabled)
                    connectBtn.clicked()
            }
        }

        SpinnerButton {
            id: connectBtn

            objectName: "importFromBackupPageConnectBtn"

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: errorLabel.visible ? 0 : JamiTheme.wizardViewPageBackButtonMargins

            preferredWidth: JamiTheme.wizardButtonWidth

            spinnerTriggeredtext: JamiStrings.generatingAccount
            normalText: JamiStrings.connectFromBackup

            enabled: {
                if (spinnerTriggered)
                    return false
                if (!(filePath.length === 0) && errorText.length === 0)
                    return true
                return false
            }

            KeyNavigation.tab: {
                if (backButton.visible)
                    return backButton
                return fileImportBtn
            }
            KeyNavigation.up: passwordFromBackupEdit
            KeyNavigation.down: KeyNavigation.tab

            onClicked: {
                if (connectBtn.focus)
                    fileImportBtn.forceActiveFocus()
                spinnerTriggered = true

                WizardViewStepModel.accountCreationInfo =
                        JamiQmlUtils.setUpAccountCreationInputPara(
                            {archivePath : UtilsAdapter.getAbsPath(filePath),
                             password : passwordFromBackupEdit.text})
                WizardViewStepModel.nextStep()
            }
        }

        Label {
            id: errorLabel

            objectName: "errorLabel"

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: JamiTheme.wizardViewPageBackButtonMargins

            visible: errorText.length !== 0

            text: errorText
            font.pointSize: JamiTheme.textFontSize
            color: JamiTheme.redColor
        }
    }

    BackButton {
        id: backButton

        objectName: "importFromBackupPageBackButton"

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        visible: !connectBtn.spinnerTriggered

        preferredSize: JamiTheme.wizardViewPageBackButtonSize

        KeyNavigation.tab: fileImportBtn
        KeyNavigation.up: {
            if (connectBtn.enabled)
                return connectBtn
            return passwordFromBackupEdit
        }
        KeyNavigation.down: KeyNavigation.tab

        onClicked: WizardViewStepModel.previousStep()
    }
}
