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
import net.jami.Enums 1.1

import "../../commoncomponents"
import "../../settingsview/components"

Rectangle {
    id: root

    property int preferredHeight: backupKeysPageColumnLayout.implicitHeight

    signal showThisPage

    function showBackupStatusDialog(success) {
        var title = success ? JamiStrings.success : JamiStrings.error
        var info = success ? JamiStrings.backupSuccessful : JamiStrings.backupFailed

        msgDialog.openWithParameters(title, info)
    }

    Connections {
        target: WizardViewStepModel

        function onMainStepChanged() {
            if (WizardViewStepModel.mainStep === WizardViewStepModel.MainSteps.BackupKeys)
                root.showThisPage()
        }
    }

    SimpleMessageDialog {
        id: msgDialog

        buttonTitles: [JamiStrings.optionOk]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue]

        onClosed: {
            if (title === JamiStrings.success)
                WizardViewStepModel.nextStep()
        }
    }

    PasswordDialog {
        id: passwordDialog

        visible: false
        purpose: PasswordDialog.ExportAccount

        onDoneSignal: function (success) {
            showBackupStatusDialog(success)
        }
    }

    // JamiFileDialog for exporting account
    JamiFileDialog {
        id: exportDialog

        mode: JamiFileDialog.SaveFile

        title: JamiStrings.backupAccountHere
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/Desktop"

        nameFilters: [JamiStrings.jamiArchiveFiles + " (*.gz)", JamiStrings.allFiles + " (*)"]

        onAccepted: {
            // Is there password? If so, go to password dialog, else, go to following directly
            if (AccountAdapter.hasPassword()) {
                passwordDialog.path = UtilsAdapter.getAbsPath(file)
                passwordDialog.open()
            } else {
                if (file.toString().length > 0)
                    showBackupStatusDialog(AccountAdapter.exportToFile(
                                               LRCInstance.currentAccountId,
                                               UtilsAdapter.getAbsPath(file)))
            }
        }

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

        onRejected: {
            backupBtn.forceActiveFocus()
        }
    }

    color: JamiTheme.backgroundColor

    ColumnLayout {
        id: backupKeysPageColumnLayout

        spacing: JamiTheme.wizardViewPageLayoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            spacing: JamiTheme.wizardViewPageLayoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredWidth: backupBtn.width

            Label {
                text: JamiStrings.backupAccount
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize + 3
            }

            BubbleLabel {
                Layout.alignment: Qt.AlignRight

                text: JamiStrings.recommended
            }
        }

        Label {
            property int preferredHeight: 0

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: backupBtn.width
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.backupAccountInfos
            color: JamiTheme.textColor
            wrapMode: Text.WordWrap
            font.pointSize: JamiTheme.textFontSize

            onFontChanged: {
                var boundingRect = JamiQmlUtils.getTextBoundingRect(font, text)
                preferredHeight = (boundingRect.width / backupBtn.preferredWidth)
                        * boundingRect.height
            }
        }

        RowLayout {
            spacing: JamiTheme.wizardViewPageLayoutSpacing

            Layout.alignment: Qt.AlignCenter

            Label {
                text: JamiStrings.neverShowAgain
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize
            }

            JamiSwitch {
                id: neverShowMeAgainSwitch

                objectName: "neverShowMeAgainSwitch"

                Layout.alignment: Qt.AlignRight

                focus: visible

                KeyNavigation.tab: backupBtn
                KeyNavigation.up: skipBackupBtn
                KeyNavigation.down: KeyNavigation.tab

                onToggled: AppSettingsManager.setValue(Settings.NeverShowMeAgain, checked)
            }
        }

        MaterialButton {
            id: backupBtn

            objectName: "backupKeyPageBackupBtn"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.backupAccountBtn
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            KeyNavigation.tab: skipBackupBtn
            KeyNavigation.up: neverShowMeAgainSwitch
            KeyNavigation.down: KeyNavigation.tab

            onClicked: exportDialog.open()
        }

        MaterialButton {
            id: skipBackupBtn

            objectName: "backupKeyPageSkipBackupBtn"

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: JamiTheme.wizardViewPageBackButtonMargins

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.skip
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed
            outlined: true

            KeyNavigation.tab: neverShowMeAgainSwitch
            KeyNavigation.up: backupBtn
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.nextStep()
        }
    }
}
