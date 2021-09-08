/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

BaseDialog {
    id: root

    property string registerdName : ""

    signal accepted

    function openNameRegistrationDialog(registerNameIn) {
        registerdName = registerNameIn
        lblRegistrationError.text = JamiStrings.somethingWentWrong
        passwordEdit.clear()

        open()

        if(AccountAdapter.hasPassword()){
            stackedWidget.currentIndex = nameRegisterEnterPasswordPage.pageIndex
            passwordEdit.forceActiveFocus()
        } else {
            startRegistration()
        }
    }

    function startRegistration() {
        stackedWidget.currentIndex = nameRegisterSpinnerPage.pageIndex
        spinnerMovie.visible = true

        timerForStartRegistration.restart()
    }

    Timer {
        id: timerForStartRegistration

        interval: 100
        repeat: false

        onTriggered: {
            AccountAdapter.model.registerName(LRCInstance.currentAccountId,
                                              passwordEdit.text, registerdName)
        }
    }

    Connections{
        target: NameDirectory

        function onNameRegistrationEnded(status, name) {
            switch(status) {
            case NameDirectory.RegisterNameStatus.SUCCESS:
                accepted()
                close()
                return
            case NameDirectory.RegisterNameStatus.WRONG_PASSWORD:
                lblRegistrationError.text = JamiStrings.incorrectPassword
                break
            case NameDirectory.RegisterNameStatus.NETWORK_ERROR:
                lblRegistrationError.text = JamiStrings.networkError
                break
            default:
                break
            }

            stackedWidget.currentIndex = nameRegisterErrorPage.pageIndex
        }
    }

    title: JamiStrings.setUsername

    contentItem: Rectangle {
        id: nameRegistrationContentRect

        implicitWidth: JamiTheme.preferredDialogWidth
        implicitHeight: JamiTheme.preferredDialogHeight

        color: JamiTheme.primaryBackgroundColor

        StackLayout {
            id: stackedWidget

            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize

            // Index = 0
            Item {
                id: nameRegisterEnterPasswordPage

                readonly property int pageIndex: 0

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 16

                    Label {
                        Layout.alignment: Qt.AlignCenter

                        text: JamiStrings.enterAccountPassword
                        color: JamiTheme.textColor
                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MaterialLineEdit {
                        id: passwordEdit

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.preferredHeight: 48

                        echoMode: TextInput.Password
                        placeholderText: JamiStrings.password

                        onTextChanged: btnRegister.enabled = (text.length > 0)

                        onAccepted: btnRegister.clicked()
                    }

                    RowLayout {
                        spacing: 16
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: true

                        MaterialButton {
                            id: btnRegister

                            Layout.alignment: Qt.AlignHCenter

                            preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                            preferredHeight: JamiTheme.preferredFieldHeight

                            color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                            hoveredColor: JamiTheme.buttonTintedBlackHovered
                            pressedColor: JamiTheme.buttonTintedBlackPressed
                            outlined: true
                            enabled: false

                            text: JamiStrings.register

                            onClicked: startRegistration()
                        }

                        MaterialButton {
                            id: btnCancel

                            Layout.alignment: Qt.AlignHCenter

                            preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                            preferredHeight: JamiTheme.preferredFieldHeight

                            color: JamiTheme.buttonTintedBlack
                            hoveredColor: JamiTheme.buttonTintedBlackHovered
                            pressedColor: JamiTheme.buttonTintedBlackPressed
                            outlined: true

                            text: JamiStrings.optionCancel

                            onClicked: close()
                        }
                    }
                }
            }

            // Index = 1
            Item {
                id: nameRegisterSpinnerPage

                readonly property int pageIndex: 1

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 16

                    Label {
                        Layout.alignment: Qt.AlignCenter

                        text: JamiStrings.registeringName
                        color: JamiTheme.textColor
                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    AnimatedImage {
                        id: spinnerMovie

                        Layout.alignment: Qt.AlignCenter

                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30

                        source: JamiResources.jami_rolling_spinner_gif
                        playing: visible
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                }
            }

            // Index = 2
            Item {
                id: nameRegisterErrorPage

                readonly property int pageIndex: 2

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 16

                    Label {
                        id: lblRegistrationError

                        Layout.alignment: Qt.AlignCenter
                        text: JamiStrings.somethingWentWrong
                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true
                        color: JamiTheme.redColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MaterialButton {
                        id: btnClose

                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

                        preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                        preferredHeight: JamiTheme.preferredFieldHeight

                        color: JamiTheme.buttonTintedBlack
                        hoveredColor: JamiTheme.buttonTintedBlackHovered
                        pressedColor: JamiTheme.buttonTintedBlackPressed
                        outlined: true

                        text: JamiStrings.close

                        onClicked: close()
                    }
                }
            }
        }
    }
}
