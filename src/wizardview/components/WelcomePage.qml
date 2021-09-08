/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
 * Author: Sébastien blin <sebastien.blin@savoirfairelinux.com>
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

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property int preferredHeight: welcomePageColumnLayout.implicitHeight

    signal scrollToBottom
    signal showThisPage

    color: JamiTheme.transparentColor

    Connections {
        target: WizardViewStepModel

        function onMainStepChanged() {
            if (WizardViewStepModel.mainStep === WizardViewStepModel.MainSteps.Initial)
                root.showThisPage()
        }
    }

    // Make sure that welcomePage grab activeFocus initially (when there is no account)
    onVisibleChanged: {
        if (visible)
            forceActiveFocus()
    }

    KeyNavigation.tab: newAccountButton
    KeyNavigation.up: newAccountButton
    KeyNavigation.down: KeyNavigation.tab

    ColumnLayout {
        id: welcomePageColumnLayout

        anchors.centerIn: parent

        spacing: JamiTheme.wizardViewPageLayoutSpacing

        Text {
            id: welcomeLabel

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredHeight: contentHeight

            text: JamiStrings.welcomeTo
            color: JamiTheme.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.pointSize: JamiTheme.welcomeLabelPointSize
            font.kerning: true
        }

        ResponsiveImage {
            id: welcomeLogo

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: JamiTheme.welcomeLogoWidth
            Layout.preferredHeight: JamiTheme.welcomeLogoHeight

            source: JamiTheme.darkTheme ?
                        JamiResources.logo_jami_standard_coul_white_svg :
                        JamiResources.logo_jami_standard_coul_svg
        }

        MaterialButton {
            id: newAccountButton

            objectName: "newAccountButton"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.createAJamiAccount
            font.capitalization: Font.AllUppercase
            toolTipText: JamiStrings.createNewJamiAccount
            iconSource: JamiResources.default_avatar_overlay_svg

            KeyNavigation.tab: newRdvButton
            KeyNavigation.up: backButton.visible ? backButton :
                                                   (showAdvancedButton.showAdvanced ?
                                                        newSIPAccountButton :
                                                        showAdvancedButton)
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.startAccountCreationFlow(
                           WizardViewStepModel.AccountCreationOption.CreateJamiAccount)
        }

        MaterialButton {
            id: newRdvButton

            objectName: "newRdvButton"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.createRV
            font.capitalization: Font.AllUppercase
            toolTipText: JamiStrings.createNewRV
            iconSource: JamiResources.groups_24dp_svg

            KeyNavigation.tab: fromDeviceButton
            KeyNavigation.up: newAccountButton
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.startAccountCreationFlow(
                           WizardViewStepModel.AccountCreationOption.CreateRendezVous)
        }

        MaterialButton {
            id: fromDeviceButton

            objectName: "fromDeviceButton"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.linkFromAnotherDevice
            font.capitalization: Font.AllUppercase
            toolTipText: JamiStrings.importAccountFromOtherDevice
            iconSource: JamiResources.devices_24dp_svg
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            KeyNavigation.tab: fromBackupButton
            KeyNavigation.up: newRdvButton
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.startAccountCreationFlow(
                           WizardViewStepModel.AccountCreationOption.ImportFromDevice)
        }

        MaterialButton {
            id: fromBackupButton

            objectName: "fromBackupButton"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.connectFromBackup
            font.capitalization: Font.AllUppercase
            toolTipText: JamiStrings.importAccountFromBackup
            iconSource: JamiResources.backup_24dp_svg
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            KeyNavigation.tab: showAdvancedButton
            KeyNavigation.up: fromDeviceButton
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.startAccountCreationFlow(
                           WizardViewStepModel.AccountCreationOption.ImportFromBackup)
        }

        MaterialButton {
            id: showAdvancedButton

            objectName: "showAdvancedButton"

            property bool showAdvanced: false

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: newSIPAccountButton.visible ?
                                     0 : JamiTheme.wizardViewPageBackButtonMargins

            preferredWidth: JamiTheme.wizardButtonWidth

            text: JamiStrings.advancedFeatures
            font.capitalization: Font.AllUppercase
            toolTipText: showAdvanced ? JamiStrings.hideAdvancedFeatures :
                                        JamiStrings.showAdvancedFeatures
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed
            outlined: true

            hoverEnabled: true

            KeyNavigation.tab: showAdvanced ? connectAccountManagerButton :
                                              (backButton.visible ? backButton : newAccountButton)
            KeyNavigation.up: fromBackupButton
            KeyNavigation.down: KeyNavigation.tab

            onClicked: {
                showAdvanced = !showAdvanced
                connectAccountManagerButton.visible = showAdvanced
                newSIPAccountButton.visible = showAdvanced
            }
        }

        MaterialButton {
            id: connectAccountManagerButton

            objectName: "connectAccountManagerButton"

            Layout.alignment: Qt.AlignCenter

            preferredWidth: JamiTheme.wizardButtonWidth

            visible: false

            text: JamiStrings.connectJAMSServer
            font.capitalization: Font.AllUppercase
            toolTipText: JamiStrings.createFromJAMS
            iconSource: JamiResources.router_24dp_svg
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            KeyNavigation.tab: newSIPAccountButton
            KeyNavigation.up: showAdvancedButton
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.startAccountCreationFlow(
                           WizardViewStepModel.AccountCreationOption.ConnectToAccountManager)
        }

        MaterialButton {
            id: newSIPAccountButton

            objectName: "newSIPAccountButton"

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: JamiTheme.wizardViewPageBackButtonMargins

            preferredWidth: JamiTheme.wizardButtonWidth

            visible: false

            text: JamiStrings.addSIPAccount
            font.capitalization: Font.AllUppercase
            toolTipText: JamiStrings.createNewSipAccount
            iconSource: JamiResources.default_avatar_overlay_svg
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            KeyNavigation.tab: backButton.visible ? backButton : newAccountButton
            KeyNavigation.up: connectAccountManagerButton
            KeyNavigation.down: KeyNavigation.tab

            onClicked: WizardViewStepModel.startAccountCreationFlow(
                           WizardViewStepModel.AccountCreationOption.CreateSipAccount)
        }

        onHeightChanged: scrollToBottom()
    }

    BackButton {
        id: backButton

        objectName: "welcomePageBackButton"

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: JamiTheme.wizardViewPageBackButtonMargins

        Connections {
            target: LRCInstance

            function onAccountListChanged() {
                backButton.visible = UtilsAdapter.getAccountListSize()
            }
        }

        preferredSize: JamiTheme.wizardViewPageBackButtonSize

        visible: UtilsAdapter.getAccountListSize()

        KeyNavigation.tab: newAccountButton
        KeyNavigation.up: newSIPAccountButton
        KeyNavigation.down: KeyNavigation.tab

        onClicked: WizardViewStepModel.previousStep()
    }
}
