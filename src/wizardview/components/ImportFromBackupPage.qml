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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

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

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: fileImportBtnText
            toolTipText: JamiStrings.importAccountArchive
            source: JamiResources.round_folder_24dp_svg
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

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

            onTextChanged: {
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
            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        SpinnerButton {
            id: connectBtn

            objectName: "connectBtn"

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: errorLabel.visible ? 0 : JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            spinnerTriggeredtext: JamiStrings.generatingAccount
            normalText: JamiStrings.connectFromBackup

            enabled: {
                if (spinnerTriggered)
                    return false
                if (!(filePath.length === 0) && errorText.length === 0)
                    return true
                return false
            }

            onClicked: {
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

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        preferredSize: JamiTheme.wizardViewPageBackButtonSize

        onClicked: WizardViewStepModel.previousStep()
    }
}
