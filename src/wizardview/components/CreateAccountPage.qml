/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14

import "../"
import "../../constant"
import "../../commoncomponents"

ColumnLayout {
    property alias text_fullNameEditAlias: fullNameEdit.text
    property alias text_usernameEditAlias: usernameEdit.text

    property int nameRegistrationUIState: WizardView.BLANK

    property alias checkState_signUpCheckboxAlias: signUpCheckbox.checked
    property alias isToSetPassword_checkState_choosePasswordCheckBox: choosePasswordCheckBox.checked

    // photo booth alias
    property alias boothImgBase64: setAvatarWidget.imgBase64

    // collapse password widget property aliases
    property alias text_passwordEditAlias: collapsiblePasswordWidget.text_passwordEditAlias
    property alias text_confirmPasswordEditAlias: collapsiblePasswordWidget.text_confirmPasswordEditAlias
    property alias displayState_passwordStatusLabelAlias: collapsiblePasswordWidget.state_passwordStatusLabelAlias

    signal validateWizardProgressionCreateAccountPage

    function initializeOnShowUp() {
        clearAllTextFields()

        signUpCheckbox.checked = true
        choosePasswordCheckBox.checked = false
        usernameEdit.enabled = true
        fullNameEdit.enabled = true
    }

    function clearAllTextFields() {
        usernameEdit.clear()
        fullNameEdit.clear()

        collapsiblePasswordWidget.clearAllTextFields()
    }

    function setCollapsiblePasswordWidgetVisibility(visible) {
        choosePasswordCheckBox.checked = visible
        if (visible) {
            choosePasswordCheckBox.visible = true
        }
    }

    function startBooth(){
        setAvatarWidget.startBooth()
    }

    function stopBooth(){
        setAvatarWidget.stopBooth()
    }

    Layout.fillWidth: true
    Layout.fillHeight: true

    spacing: 6

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    ColumnLayout {
        Layout.alignment: Qt.AlignHCenter

        spacing: 5

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Layout.alignment: Qt.AlignHCenter

        Label {
            id: profileSectionLabel


            Layout.alignment: Qt.AlignHCenter

            text: qsTr("Profile")
            font.pointSize: 13
            font.kerning: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

    PhotoboothView{
        id: setAvatarWidget

        Layout.alignment: Qt.AlignHCenter

        Layout.maximumWidth: 261
        Layout.preferredWidth: 261
        Layout.minimumWidth: 261
        Layout.maximumHeight: 261
        Layout.preferredHeight: 261
        Layout.minimumHeight: 261
    }

        RowLayout {
            spacing: 6
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumHeight: 30

            Item {
                Layout.fillWidth: true
                Layout.maximumHeight: 10
            }

            InfoLineEdit {
                id: fullNameEdit

                fieldLayoutWidth: 261

                Layout.alignment: Qt.AlignCenter

                selectByMouse: true
                placeholderText: qsTr("Profile name")
                font.pointSize: 10
                font.kerning: true
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    ColumnLayout {
        Layout.alignment: Qt.AlignHCenter

        spacing: 5
        Label {
            id: accountSectionLabel
            Layout.alignment: Qt.AlignHCenter

            Layout.maximumWidth: 261
            Layout.preferredWidth: 261
            Layout.minimumWidth: 261
            Layout.maximumHeight: 30
            Layout.preferredHeight: 30
            Layout.minimumHeight: 30

            text: qsTr("Account")
            font.pointSize: 13
            font.kerning: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            CheckBox {
                id: signUpCheckbox
                checked: true

                indicator.width: 10
                indicator.height: 10

                Layout.leftMargin: 32

                Layout.minimumWidth: 261

                Layout.maximumHeight: 30
                Layout.preferredHeight: 30
                Layout.minimumHeight: 25

                Layout.alignment: Qt.AlignLeft

                text: qsTr("Register public username")
                font.pointSize: 10
                font.kerning: true

                indicator.implicitWidth: 20
                indicator.implicitHeight:20

                onClicked: {
                    if (!checked) {
                        usernameEdit.clear()
                    }

                    validateWizardProgressionCreateAccountPage()
                }
            }
        }

        RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Layout.leftMargin: 32

            InfoLineEdit {
                id: usernameEdit

                fieldLayoutWidth: 261

                Layout.alignment: Qt.AlignHCenter

                selectByMouse: true
                placeholderText: qsTr("Choose your username")
                font.pointSize: 10
                font.kerning: true

                enabled: signUpCheckbox.visible && signUpCheckbox.checked
            }

            LookupStatusLabel{
                id: lookupStatusLabel

                visible: true

                lookupStatusState: {
                    switch (nameRegistrationUIState) {
                    case WizardView.BLANK:
                        return "Blank"
                    case WizardView.INVALID:
                        return "Invalid"
                    case WizardView.TAKEN:
                        return "Taken"
                    case WizardView.FREE:
                        return "Free"
                    case WizardView.SEARCHING:
                        return "Searching"
                    default:
                        return "Blank"
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            CheckBox {
                id: choosePasswordCheckBox
                checked: false

                indicator.width: 10
                indicator.height: 10

                Layout.leftMargin: 32

                Layout.minimumWidth: 261

                Layout.preferredHeight: 30
                Layout.minimumHeight: 25

                indicator.implicitWidth: 20
                indicator.implicitHeight:20

                Layout.alignment: Qt.AlignLeft

                text: qsTr("Choose a password for enhanced security")
                font.pointSize: 8
                font.kerning: true

                onClicked: {
                    if (!checked) {
                        collapsiblePasswordWidget.clearAllTextFields()
                    }

                    validateWizardProgressionCreateAccountPage()
                }
            }

            CollapsiblePasswordWidget {
                id: collapsiblePasswordWidget

                Layout.alignment: Qt.AlignHCenter

                visibleCollapsble: choosePasswordCheckBox.checked
                                   && choosePasswordCheckBox.visible
            }
        }

        Item {
            Layout.maximumWidth: 261
            Layout.preferredWidth: 261
            Layout.minimumWidth: 261

            Layout.maximumHeight: 30
            Layout.preferredHeight: 30
            Layout.minimumHeight: 30

            Layout.alignment: Qt.AlignHCenter
        }
    }
}
