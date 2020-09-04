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

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0

import "../../commoncomponents"

Dialog {
    id: nameRegistrationDialog

    property string registerdName : ""

    function openNameRegistrationDialog(registerNameIn){
        registerdName = registerNameIn
        lblRegistrationError.text = qsTr("Something went wrong")
        passwordEdit.clear()
        if(ClientWrapper.accountAdaptor.hasPassword()){
            stackedWidget.currentIndex = 0
        } else {
            startRegistration()
        }

        nameRegistrationDialog.open()
    }

    function startRegistration(){
        startSpinner()
        timerForStartRegistration.restart()
    }

    function slotStartNameRegistration(){
        var password = passwordEdit.text
        ClientWrapper.accountModel.registerName(UtilsAdapter.getCurrAccId(), password, registerdName)
    }

    function startSpinner(){
        stackedWidget.currentIndex = 1
        spinnerLabel.visible = true
        spinnerMovie.playing = true
    }

    Timer{
        id: timerForStartRegistration

        interval: 100
        repeat: false

        onTriggered: {
            slotStartNameRegistration()
        }
    }

    Connections{
        target: ClientWrapper.nameDirectory

        function onNameRegistrationEnded(status, name){
            if(status === NameDirectory.RegisterNameStatus.SUCCESS){
                accept()
            } else {
                switch(status){
                case NameDirectory.RegisterNameStatus.WRONG_PASSWORD:
                    lblRegistrationError.text = qsTr("Incorrect password")
                    break

                case NameDirectory.RegisterNameStatus.NETWORK_ERROR:
                    lblRegistrationError.text = qsTr("Network error")
                    break
                default:
                    break
                }
                stackedWidget.currentIndex = 2
            }
        }
    }

    visible: false

    anchors.centerIn: parent.Center
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    title: qsTr("Set Registered Name")

    onClosed: {
        reject()
    }

    contentItem: Rectangle{
        implicitWidth: 350
        implicitHeight: 208

        StackLayout{
            id: stackedWidget
            anchors.fill: parent

            currentIndex: 0

            Rectangle{
                id: passwordConfirmPage

                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.topMargin: 11
                Layout.bottomMargin: 11

                ColumnLayout{
                    anchors.fill: parent
                    spacing: 7

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true
                        Layout.maximumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.minimumHeight: 20
                    }

                    Label{
                        Layout.preferredWidth: 219
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.Wrap
                        text: qsTr("Enter your account password")
                        font.pointSize: 8
                        font.kerning: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true

                        Layout.maximumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.minimumHeight: 20
                    }

                    InfoLineEdit{
                        id: passwordEdit

                        Layout.alignment: Qt.AlignHCenter

                        Layout.minimumWidth: 294
                        Layout.preferredWidth: 294

                        Layout.preferredHeight: 30
                        Layout.minimumHeight: 30

                        echoMode: TextInput.Password

                        placeholderText: qsTr("Password")
                    }

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true

                        Layout.maximumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.minimumHeight: 20
                    }

                    RowLayout{
                        spacing: 7

                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillWidth: true

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        HoverableRadiusButton{
                            id: btnRegister

                            Layout.maximumWidth: 130
                            Layout.preferredWidth: 130
                            Layout.minimumWidth: 130

                            Layout.maximumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.minimumHeight: 30

                            radius: height /2

                            text: qsTr("Register")
                            font.pointSize: 10
                            font.kerning: true

                            onClicked: {
                                startRegistration()
                            }
                        }

                        Item{
                            Layout.fillWidth: true
                            Layout.minimumWidth: 40

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        HoverableButtonTextItem {
                            id: btnCancel

                            Layout.maximumWidth: 130
                            Layout.preferredWidth: 130
                            Layout.minimumWidth: 130

                            Layout.maximumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.minimumHeight: 30

                            backgroundColor: "red"
                            onEnterColor: Qt.rgba(150 / 256, 0, 0, 0.7)
                            onDisabledBackgroundColor: Qt.rgba(
                                                           255 / 256,
                                                           0, 0, 0.8)
                            onPressColor: backgroundColor
                            textColor: "white"

                            radius: height /2

                            text: qsTr("Cancel")
                            font.pointSize: 10
                            font.kerning: true

                            onClicked: {
                                reject()
                            }
                        }

                        Item{
                            Layout.fillWidth: true
                            Layout.minimumWidth: 40

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }
                    }

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true

                        Layout.maximumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.minimumHeight: 20
                    }
                }
            }

            Rectangle{
                id: registeringPage

                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.topMargin: 11
                Layout.bottomMargin: 11

                ColumnLayout{
                    anchors.fill: parent
                    spacing: 7

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true
                        Layout.minimumHeight: 40

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }

                    RowLayout{
                        Layout.fillWidth: true
                        spacing: 0

                        Layout.maximumHeight: 30

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        Label{
                            Layout.alignment: Qt.AlignHCenter

                            Layout.maximumWidth: 0
                            Layout.preferredWidth: 341

                            Layout.minimumHeight: 0
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            wrapMode: Text.Wrap
                            text: qsTr("Registering Name")
                            font.pointSize: 8
                            font.kerning: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }
                    }

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }

                    RowLayout{
                        spacing: 7

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        Label{
                            id: spinnerLabel

                            Layout.alignment: Qt.AlignHCenter

                            Layout.maximumWidth: 96
                            Layout.preferredWidth: 96
                            Layout.minimumWidth: 96

                            Layout.maximumHeight: 96
                            Layout.preferredHeight: 96
                            Layout.minimumHeight: 96

                            background: Rectangle {
                                anchors.fill: parent
                                AnimatedImage {
                                    id: spinnerMovie

                                    anchors.fill: parent

                                    source: "qrc:/images/jami_eclipse_spinner.gif"

                                    playing: spinnerLabel.visible
                                    paused: false
                                    fillMode: Image.PreserveAspectFit
                                    mipmap: true
                                }
                            }
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                    }

                    Item{
                        Layout.alignment: Qt.AlignHCenter

                        Layout.fillHeight: true
                        Layout.minimumHeight: 40

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }
                }
            }

            Rectangle{
                id: nameNotRegisteredPage

                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout{
                    anchors.fill: parent

                    Item{
                        Layout.fillHeight: true
                        Layout.minimumHeight: 40

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }

                    RowLayout{
                        spacing:  7
                        Layout.fillWidth: true

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        Label{
                            id: lblRegistrationError

                            Layout.alignment: Qt.AlignHCenter

                            Layout.maximumWidth: 0
                            Layout.preferredWidth: 341

                            Layout.minimumHeight: 0
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            wrapMode: Text.Wrap
                            text: qsTr("Something went wrong")
                            font.pointSize: 8
                            font.kerning: true
                            color: "red"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }
                    }

                    Item{
                        Layout.fillHeight: true
                        Layout.minimumHeight: 40

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }

                    RowLayout{
                        spacing:  7
                        Layout.fillWidth: true

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        HoverableRadiusButton{
                            id: btnCloseRegisterDialog

                            Layout.maximumWidth: 130
                            Layout.preferredWidth: 130
                            Layout.minimumWidth: 130

                            Layout.maximumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.minimumHeight: 30

                            radius: height /2

                            text: qsTr("Close")
                            font.pointSize: 10
                            font.kerning: true

                            onClicked: {
                                reject()
                            }
                        }

                        Item{
                            Layout.fillWidth: true

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }
                    }

                    Item{
                        Layout.fillHeight: true
                        Layout.minimumHeight: 40

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }
                }
            }
        }
    }
}
