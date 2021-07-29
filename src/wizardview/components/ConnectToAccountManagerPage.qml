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

import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property int preferredHeight: connectToAccountManagerPageColumnLayout.implicitHeight
    property string errorText: ""

    signal showThisPage

    function clearAllTextFields() {
        connectBtn.spinnerTriggered = false
        usernameManagerEdit.clear()
        passwordManagerEdit.clear()
        accountManagerEdit.clear()
        errorText = ""
    }

    function errorOccured(errorMessage) {
        connectBtn.spinnerTriggered = false
        errorText = errorMessage
    }

    Connections {
        target: WizardViewStepModel

        function onMainStepChanged() {
            if (WizardViewStepModel.mainStep === WizardViewStepModel.MainSteps.AccountCreation &&
                    WizardViewStepModel.accountCreationOption ===
                    WizardViewStepModel.AccountCreationOption.ConnectToAccountManager) {
                clearAllTextFields()
                root.showThisPage()
            }
        }
    }

    color: JamiTheme.backgroundColor

    onVisibleChanged: {
        if (visible)
            accountManagerEdit.focus = true
    }

    ColumnLayout {
        id: connectToAccountManagerPageColumnLayout

        spacing: JamiTheme.wizardViewPageLayoutSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            spacing: JamiTheme.wizardViewPageLayoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredWidth: implicitWidth

            Label {
                text: JamiStrings.enterJAMSURL
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize + 3
            }

            BubbleLabel {
                Layout.alignment: Qt.AlignRight

                text: JamiStrings.required
                textColor: JamiTheme.requiredFieldColor
                bubbleColor: JamiTheme.requiredFieldBackgroundColor
            }
        }

        MaterialLineEdit {
            id: accountManagerEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: JamiStrings.jamiManagementServerURL
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: connectBtn.width

            text: JamiStrings.jamsCredentials
            color: JamiTheme.textColor
            wrapMode: Text.Wrap

            onTextChanged: Layout.preferredHeight =
                           JamiQmlUtils.getTextBoundingRect(font, text).height
        }

        MaterialLineEdit {
            id: usernameManagerEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: JamiStrings.username
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            onTextChanged: errorText = ""
        }

        MaterialLineEdit {
            id: passwordManagerEdit

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: connectBtn.width
            Layout.alignment: Qt.AlignCenter

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

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: errorLabel.visible ? 0 : JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            spinnerTriggeredtext: JamiStrings.creatingAccount
            normalText: JamiStrings.connect

            enabled: accountManagerEdit.text.length !== 0
                     && usernameManagerEdit.text.length !== 0
                     && passwordManagerEdit.text.length !== 0
                     && !spinnerTriggered

            onClicked: {
                spinnerTriggered = true

                WizardViewStepModel.accountCreationInfo =
                        JamiQmlUtils.setUpAccountCreationInputPara(
                            {username : usernameManagerEdit.text,
                             password : passwordManagerEdit.text,
                             manager : accountManagerEdit.text})
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

        objectName: "connectToAccountManagerPageBackButton"

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        visible: !connectBtn.spinnerTriggered

        preferredSize: JamiTheme.wizardViewPageBackButtonSize

        onClicked: WizardViewStepModel.previousStep()
    }
}
