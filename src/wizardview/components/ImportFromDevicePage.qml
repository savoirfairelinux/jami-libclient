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

import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property string errorText: ""
    property int preferredHeight: importFromDevicePageColumnLayout.implicitHeight

    signal showThisPage

    function initializeOnShowUp() {
        clearAllTextFields()
    }

    function clearAllTextFields() {
        connectBtn.spinnerTriggered = false
        pinFromDevice.clear()
        passwordFromDevice.clear()
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
                    WizardViewStepModel.AccountCreationOption.ImportFromDevice) {
                clearAllTextFields()
                root.showThisPage()
            }
        }
    }

    color: JamiTheme.backgroundColor

    ColumnLayout {
        id: importFromDevicePageColumnLayout

        spacing: JamiTheme.wizardViewPageLayoutSpacing

        // Prevent possible anchor loop detected on centerIn.
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Text {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins

            text: JamiStrings.mainAccountPassword
            color: JamiTheme.textColor
            font.pointSize: JamiTheme.menuFontSize
        }

        MaterialLineEdit {
            id: passwordFromDevice

            objectName: "passwordFromDevice"

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: JamiStrings.password
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true

            echoMode: TextInput.Password

            KeyNavigation.tab: pinFromDevice
            KeyNavigation.up: {
                if (backButton.visible)
                    return backButton
                return pinFromDevice
            }
            KeyNavigation.down: KeyNavigation.tab

            onTextChanged: errorText = ""
            onAccepted: pinFromDevice.forceActiveFocus()
        }

        Text {
            property int preferredHeight: JamiTheme.wizardViewPageLayoutSpacing


            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: connectBtn.width
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.enterPIN
            color: JamiTheme.textColor
            wrapMode: Text.Wrap

            onTextChanged: function (text) {
                var boundingRect = JamiQmlUtils.getTextBoundingRect(font, text)
                preferredHeight += (boundingRect.width / connectBtn.preferredWidth)
                        * boundingRect.height
            }
        }

        MaterialLineEdit {
            id: pinFromDevice

            objectName: "pinFromDevice"

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            focus: visible

            selectByMouse: true
            placeholderText: JamiStrings.pin
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true

            KeyNavigation.tab: {
                if (connectBtn.enabled)
                    return connectBtn
                else if (connectBtn.spinnerTriggered)
                    return passwordFromDevice
                return backButton
            }
            KeyNavigation.up: passwordFromDevice
            KeyNavigation.down: KeyNavigation.tab

            onTextChanged: errorText = ""
            onAccepted: {
                if (connectBtn.enabled)
                    connectBtn.clicked()
            }
        }

        SpinnerButton {
            id: connectBtn

            objectName: "importFromDevicePageConnectBtn"

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: errorLabel.visible ? 0 : JamiTheme.wizardViewPageBackButtonMargins

            preferredWidth: JamiTheme.wizardButtonWidth

            spinnerTriggeredtext: JamiStrings.generatingAccount
            normalText: JamiStrings.connectFromAnotherDevice

            enabled: pinFromDevice.text.length !== 0 && !spinnerTriggered

            KeyNavigation.tab: backButton
            KeyNavigation.up: pinFromDevice
            KeyNavigation.down: KeyNavigation.tab

            onClicked: {
                spinnerTriggered = true

                WizardViewStepModel.accountCreationInfo =
                        JamiQmlUtils.setUpAccountCreationInputPara(
                            {archivePin : pinFromDevice.text,
                             password : passwordFromDevice.text})
                WizardViewStepModel.nextStep()
            }
        }

        Label {
            id: errorLabel

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

        objectName: "importFromDevicePageBackButton"

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        visible: !connectBtn.spinnerTriggered

        KeyNavigation.tab: passwordFromDevice
        KeyNavigation.up: connectBtn
        KeyNavigation.down: KeyNavigation.tab

        preferredSize: JamiTheme.wizardViewPageBackButtonSize

        onClicked: WizardViewStepModel.previousStep()
    }
}
