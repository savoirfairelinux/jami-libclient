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

    property int preferredHeight: createSIPAccountPageColumnLayout.implicitHeight

    signal showThisPage

    function clearAllTextFields() {
        sipUsernameEdit.clear()
        sipPasswordEdit.clear()
        sipServernameEdit.clear()
        sipProxyEdit.clear()
        sipUsernameEdit.clear()
    }

    Connections {
        target: WizardViewStepModel

        function onMainStepChanged() {
            if (WizardViewStepModel.mainStep === WizardViewStepModel.MainSteps.AccountCreation &&
                    WizardViewStepModel.accountCreationOption ===
                    WizardViewStepModel.AccountCreationOption.CreateSipAccount) {
                clearAllTextFields()
                root.showThisPage()
            }
        }
    }

    color: JamiTheme.backgroundColor

    ColumnLayout {
        id: createSIPAccountPageColumnLayout

        spacing: JamiTheme.wizardViewPageLayoutSpacing

        anchors.centerIn: parent

        RowLayout {
            spacing: JamiTheme.wizardViewPageLayoutSpacing

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredWidth: createAccountButton.width

            Label {
                text: JamiStrings.configureExistingSIP
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize + 3
            }

            BubbleLabel {
                Layout.alignment: Qt.AlignRight

                text: JamiStrings.optional
                bubbleColor: JamiTheme.wizardBlueButtons
            }
        }

        MaterialLineEdit {
            id: sipServernameEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            focus: visible

            selectByMouse: true
            placeholderText: JamiStrings.server
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipProxyEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: JamiStrings.proxy
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipUsernameEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            placeholderText: JamiStrings.username
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true
        }

        MaterialLineEdit {
            id: sipPasswordEdit

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: createAccountButton.width

            selectByMouse: true
            echoMode: TextInput.Password
            placeholderText: JamiStrings.password
            font.pointSize: JamiTheme.textFontSize
            font.kerning: true
        }

        MaterialButton {
            id: createAccountButton

            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: JamiTheme.wizardViewPageBackButtonMargins
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.createSIPAccount
            color: JamiTheme.wizardBlueButtons
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                WizardViewStepModel.accountCreationInfo =
                        JamiQmlUtils.setUpAccountCreationInputPara(
                            {hostname : sipServernameEdit.text,
                             username : sipUsernameEdit.text,
                             password : sipPasswordEdit.text,
                             proxy : sipProxyEdit.text})
                WizardViewStepModel.nextStep()
            }
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
