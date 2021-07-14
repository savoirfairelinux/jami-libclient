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
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import net.jami.Adapters 1.0
import net.jami.Constants 1.0
import net.jami.Helpers 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    // trigger a default avatar prior to account generation
    property string createdAccountId: "dummy"
    property int preferredHeight: profilePageColumnLayout.implicitHeight
    property var showBottom: false
    property alias displayName: aliasEdit.text
    property bool isRdv: false
    property alias avatarBooth: setAvatarWidget
    property bool avatarSet

    signal leavePage
    signal saveProfile

    function initializeOnShowUp() {
        createdAccountId = "dummy"
        clearAllTextFields()
        saveProfileBtn.spinnerTriggered = true
        avatarSet = false
    }

    function clearAllTextFields() {
        aliasEdit.clear()
    }

    function readyToSaveDetails() {
        saveProfileBtn.spinnerTriggered = false
    }

    color: JamiTheme.backgroundColor

    ColumnLayout {
        id: profilePageColumnLayout

        spacing: layoutSpacing

        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            spacing: layoutSpacing

            Layout.topMargin: backButtonMargins
            Layout.preferredWidth: saveProfileBtn.width
            Layout.alignment: Qt.AlignCenter

            Label {
                text: qsTr("Profile is only shared with contacts")
                color: JamiTheme.textColor
                font.pointSize: JamiTheme.textFontSize + 3
            }

            Label {
                Layout.alignment: Qt.AlignRight

                text: qsTr("Optional")
                color: JamiTheme.whiteColor
                padding: 8

                background: Rectangle {
                    color: JamiTheme.wizardBlueButtons
                    radius: 24
                    anchors.fill: parent
                }
            }
        }

        PhotoboothView {
            id: setAvatarWidget

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: size
            Layout.fillHeight: true

            imageId: createdAccountId
            onAvatarSet: root.avatarSet = true

            size: 200
        }

        MaterialLineEdit {
            id: aliasEdit

            property string lastFirstChar

            Layout.preferredHeight: fieldLayoutHeight
            Layout.preferredWidth: fieldLayoutWidth
            Layout.alignment: Qt.AlignCenter

            selectByMouse: true
            placeholderText: isRdv ? JamiStrings.enterRVName : qsTr("Enter your name")
            font.pointSize: 9
            font.kerning: true

            borderColorMode: MaterialLineEdit.NORMAL

            fieldLayoutWidth: saveProfileBtn.width

            onTextEdited: {
                if (root.avatarSet)
                    return
                if (text.length === 0) {
                    lastFirstChar = ""
                    AccountAdapter.setCurrAccDisplayName(lastFirstChar)
                } else if (text.length == 1 && text.charAt(0) !== lastFirstChar) {
                    lastFirstChar = text.charAt(0)
                    AccountAdapter.setCurrAccDisplayName(lastFirstChar)
                }
            }
        }

        SpinnerButton {
            id: saveProfileBtn

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            enabled: !spinnerTriggered
            normalText: JamiStrings.saveProfile
            spinnerTriggeredtext: root.isRdv ? JamiStrings.generatingRV : qsTr("Generating account…")
            onClicked: saveProfile()
        }

        MaterialButton {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: preferredWidth
            Layout.preferredHeight: preferredHeight

            text: JamiStrings.skip
            enabled: saveProfileBtn.enabled
            color: enabled? JamiTheme.buttonTintedGrey : JamiTheme.buttonTintedGreyInactive
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed
            outlined: true

            onClicked: {
                AccountAdapter.setCurrentAccountAvatarBase64()
                aliasEdit.clear()
                saveProfile()
            }
        }

        AccountCreationStepIndicator {
            Layout.topMargin: backButtonMargins
            Layout.bottomMargin: backButtonMargins
            Layout.alignment: Qt.AlignHCenter

            spacing: layoutSpacing
            steps: 3
            currentStep: 3
        }
    }
}
