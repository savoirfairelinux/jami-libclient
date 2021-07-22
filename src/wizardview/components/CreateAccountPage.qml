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

import "../"
import "../../commoncomponents"
import "../../settingsview/components"

Rectangle {
    id: root

    property bool isRendezVous: false
    property int preferredHeight: {
        if (createAccountStack.currentIndex === 0)
            return usernameColumnLayout.implicitHeight
        return passwordColumnLayout.implicitHeight
    }

    signal showThisPage

    function initializeOnShowUp(isRdv) {
        isRendezVous = isRdv
        createAccountStack.currentIndex = 0
        clearAllTextFields()
        passwordSwitch.checked = false
    }

    function clearAllTextFields() {
        usernameEdit.clear()
        passwordEdit.clear()
        passwordConfirmEdit.clear()
    }

    color: JamiTheme.backgroundColor

    Connections {
        target: WizardViewStepModel

        function onMainStepChanged() {
            var currentMainStep = WizardViewStepModel.mainStep
            if (currentMainStep === WizardViewStepModel.MainSteps.NameRegistration) {
                createAccountStack.currentIndex = nameRegistrationPage.stackIndex
                initializeOnShowUp(WizardViewStepModel.accountCreationOption ===
                                   WizardViewStepModel.AccountCreationOption.CreateRendezVous)
                root.showThisPage()
            } else if (currentMainStep === WizardViewStepModel.MainSteps.SetPassword) {
                createAccountStack.currentIndex = passwordSetupPage.stackIndex
            }
        }
    }

    StackLayout {
        id: createAccountStack

        objectName: "createAccountStack"

        anchors.fill: parent

        Rectangle {
            id: nameRegistrationPage

            objectName: "nameRegistrationPage"

            property int stackIndex: 0

            color: JamiTheme.backgroundColor

            ColumnLayout {
                id: usernameColumnLayout

                spacing: JamiTheme.wizardViewPageLayoutSpacing

                anchors.centerIn: parent

                width: root.width

                RowLayout {
                    spacing: JamiTheme.wizardViewPageLayoutSpacing

                    Layout.alignment: Qt.AlignCenter
                    Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
                    Layout.preferredWidth: usernameEdit.width

                    Label {
                        text: isRendezVous ? JamiStrings.chooseUsernameForRV :
                                             JamiStrings.chooseUsernameForAccount
                        color: JamiTheme.textColor
                        font.pointSize: JamiTheme.textFontSize + 3
                    }

                    BubbleLabel {
                        Layout.alignment: Qt.AlignRight

                        text: JamiStrings.recommended
                    }
                }

                UsernameLineEdit {
                    id: usernameEdit

                    objectName: "usernameEdit"

                    Layout.topMargin: 15
                    Layout.preferredHeight: fieldLayoutHeight
                    Layout.preferredWidth:  chooseUsernameButton.width
                    Layout.alignment: Qt.AlignHCenter

                    focus: visible

                    placeholderText: isRendezVous ? JamiStrings.chooseAName :
                                                    JamiStrings.chooseYourUserName
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: text.length !==0

                    text: {
                        switch(usernameEdit.nameRegistrationState){
                        case UsernameLineEdit.NameRegistrationState.BLANK:
                        case UsernameLineEdit.NameRegistrationState.SEARCHING:
                        case UsernameLineEdit.NameRegistrationState.FREE:
                            return ""
                        case UsernameLineEdit.NameRegistrationState.INVALID:
                            return isRendezVous ? JamiStrings.invalidName :
                                                  JamiStrings.invalidUsername
                        case UsernameLineEdit.NameRegistrationState.TAKEN:
                            return isRendezVous ? JamiStrings.nameAlreadyTaken :
                                                  JamiStrings.usernameAlreadyTaken
                        }
                    }
                    font.pointSize: JamiTheme.textFontSize
                    color: JamiTheme.redColor
                }

                MaterialButton {
                    id: chooseUsernameButton

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredWidth
                    Layout.preferredHeight: preferredHeight

                    fontCapitalization: Font.AllUppercase
                    text: isRendezVous ? JamiStrings.chooseName : JamiStrings.chooseUsername
                    enabled: usernameEdit.nameRegistrationState === UsernameLineEdit.NameRegistrationState.FREE
                    color: usernameEdit.nameRegistrationState === UsernameLineEdit.NameRegistrationState.FREE ?
                               JamiTheme.wizardBlueButtons :
                               JamiTheme.buttonTintedGreyInactive
                    hoveredColor: JamiTheme.buttonTintedBlueHovered
                    pressedColor: JamiTheme.buttonTintedBluePressed

                    onClicked: WizardViewStepModel.nextStep()
                }

                MaterialButton {
                    id: skipButton

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredWidth
                    Layout.preferredHeight: preferredHeight

                    text: JamiStrings.skip
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed
                    outlined: true

                    onClicked: {
                        usernameEdit.clear()
                        WizardViewStepModel.nextStep()
                    }
                }

                AccountCreationStepIndicator {
                    Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
                    Layout.bottomMargin: JamiTheme.wizardViewPageBackButtonMargins
                    Layout.alignment: Qt.AlignHCenter

                    spacing: JamiTheme.wizardViewPageLayoutSpacing
                    steps: 2
                    currentStep: 1
                }
            }
        }

        Rectangle {
            id: passwordSetupPage

            objectName: "passwordSetupPage"

            property int stackIndex: 1

            color: JamiTheme.backgroundColor

            ColumnLayout {
                id: passwordColumnLayout

                spacing: JamiTheme.wizardViewPageLayoutSpacing

                anchors.centerIn: parent
                width: root.width

                RowLayout {
                    spacing: JamiTheme.wizardViewPageLayoutSpacing

                    Layout.alignment: Qt.AlignCenter
                    Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
                    Layout.preferredWidth: usernameEdit.width

                    Label {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: JamiStrings.createPassword
                        color: JamiTheme.textColor
                        font.pointSize: JamiTheme.textFontSize + 3
                    }

                    Switch {
                        id: passwordSwitch

                        objectName: "passwordSwitch"

                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        Layout.leftMargin: -JamiTheme.wizardViewPageLayoutSpacing
                        Layout.topMargin: 5
                    }

                    BubbleLabel {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                        text: JamiStrings.optional
                        bubbleColor: JamiTheme.wizardBlueButtons
                    }
                }

                MaterialLineEdit {
                    id: passwordEdit

                    objectName: "passwordEdit"

                    Layout.preferredHeight: fieldLayoutHeight
                    Layout.preferredWidth: createAccountButton.width
                    Layout.alignment: Qt.AlignHCenter

                    focus: visible
                    visible: passwordSwitch.checked

                    selectByMouse: true
                    echoMode: TextInput.Password
                    placeholderText: JamiStrings.password
                    font.pointSize: JamiTheme.textFontSize
                    font.kerning: true
                }

                MaterialLineEdit {
                    id: passwordConfirmEdit

                    objectName: "passwordConfirmEdit"

                    Layout.preferredHeight: fieldLayoutHeight
                    Layout.preferredWidth: createAccountButton.width
                    Layout.alignment: Qt.AlignHCenter

                    visible: passwordSwitch.checked

                    selectByMouse: true
                    echoMode: TextInput.Password
                    placeholderText: JamiStrings.confirmPassword
                    font.pointSize: JamiTheme.textFontSize
                    font.kerning: true
                }

                Label {
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: createAccountButton.width - 10
                    Layout.leftMargin: (root.width - createAccountButton.width) / 2

                    text: JamiStrings.notePasswordRecovery
                    color: JamiTheme.textColor
                    wrapMode: Text.WordWrap
                    font.pointSize: JamiTheme.textFontSize
                }

                MaterialButton {
                    id: createAccountButton

                    objectName: "createAccountButton"

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredWidth
                    Layout.preferredHeight: preferredHeight

                    function checkEnable() {
                        return !passwordSwitch.checked ||
                                (passwordEdit.text === passwordConfirmEdit.text
                                 && passwordEdit.text.length !== 0)
                    }

                    fontCapitalization: Font.AllUppercase
                    text: isRendezVous ? JamiStrings.createRV : JamiStrings.createAccount
                    enabled: checkEnable()
                    color: checkEnable() ? JamiTheme.wizardBlueButtons :
                                           JamiTheme.buttonTintedGreyInactive
                    hoveredColor: JamiTheme.buttonTintedBlueHovered
                    pressedColor: JamiTheme.buttonTintedBluePressed

                    onClicked: {
                        WizardViewStepModel.accountCreationInfo =
                                JamiQmlUtils.setUpAccountCreationInputPara(
                                    {isRendezVous : WizardViewStepModel.accountCreationOption ===
                                                    WizardViewStepModel.AccountCreationOption.CreateRendezVous,
                                     password : passwordEdit.text,
                                     registeredName : usernameEdit.text})
                        WizardViewStepModel.nextStep()
                    }
                }

                AccountCreationStepIndicator {
                    Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
                    Layout.bottomMargin: JamiTheme.wizardViewPageBackButtonMargins
                    Layout.alignment: Qt.AlignHCenter

                    spacing: JamiTheme.wizardViewPageLayoutSpacing
                    steps: 2
                    currentStep: 2
                }
            }
        }
    }

    BackButton {
        id: backButton

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: JamiTheme.wizardViewPageBackButtonMargins

        preferredSize: JamiTheme.wizardViewPageBackButtonSize

        onClicked: WizardViewStepModel.previousStep()
    }
}
