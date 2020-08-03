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
    id: linkDeviceDialog

    function openLinkDeviceDialog(){
        infoLabel.text = qsTr("This pin and the account password should be entered in your device within 10 minutes.")
        passwordEdit.clear()
        linkDeviceDialog.open()
        if(ClientWrapper.accountAdaptor.hasPassword()){
            stackedWidget.currentIndex = 0
        } else {
            setGeneratingPage()
        }
    }

    function setGeneratingPage(){
        if(passwordEdit.length === 0 && ClientWrapper.accountAdaptor.hasPassword()){
            setExportPage(NameDirectory.ExportOnRingStatus.WRONG_PASSWORD, "")
            return
        }

        stackedWidget.currentIndex = 1
        spinnerMovie.playing = true

        timerForExport.restart()
    }

    function slotExportOnRing(){
        ClientWrapper.accountModel.exportOnRing(ClientWrapper.utilsAdaptor.getCurrAccId(),passwordEdit.text)
    }

    Timer{
        id: timerForExport

        repeat: false
        interval: 200

        onTriggered: {
            timeOut.restart()
            slotExportOnRing()
        }
    }

    Timer{
        id: timeOut

        repeat: false
        interval: exportTimeout

        onTriggered: {
            setExportPage(NameDirectory.ExportOnRingStatus.NETWORK_ERROR, "")
        }
    }

    function setExportPage(status, pin){
        timeOut.stop()

        if(status === NameDirectory.ExportOnRingStatus.SUCCESS){
            infoLabel.isSucessState = true
            yourPinLabel.visible = true
            exportedPIN.visible = true
            infoLabel.text = qsTr("This pin and the account password should be entered in your device within 10 minutes.")
            exportedPIN.text = pin
        } else {
            infoLabel.isSucessState = false
            yourPinLabel.visible = false
            exportedPIN.visible = false

            switch(status){
            case NameDirectory.ExportOnRingStatus.WRONG_PASSWORD:
                infoLabel.text = qsTr("Incorrect password")

                break
            case NameDirectory.ExportOnRingStatus.NETWORK_ERROR:
                infoLabel.text = qsTr("Error connecting to the network.\nPlease try again later.")

                break
            case NameDirectory.ExportOnRingStatus.INVALID:
                infoLabel.text = qsTr("Something went wrong.\n")

                break
            }
        }
        stackedWidget.currentIndex = 2
    }

    property int exportTimeout : 20000

    Connections{
        target: ClientWrapper.nameDirectory

        function onExportOnRingEnded(status, pin){
            setExportPage(status, pin)
        }
    }

    visible: false

    anchors.centerIn: parent.Center
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    title: qsTr("Link another device")

    onClosed: {
        if(infoLabel.isSucessState){
            accept()
        } else {
            reject()
        }
    }

    contentItem: Rectangle{
        implicitWidth: 365
        implicitHeight: 208

        StackLayout{
            id: stackedWidget
            anchors.fill: parent

            currentIndex: 2

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
                            id: btnPasswordOk

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
                                setGeneratingPage()
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
                id: exportingPage

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
                            text: qsTr("Exporting Account")
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
                            id: exportingSpinner

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

                                    playing: exportingSpinner.visible
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
                id: exportedPage

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
                        Layout.fillHeight: true

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
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

                        Label{
                            id: yourPinLabel

                            Layout.alignment: Qt.AlignHCenter

                            Layout.preferredHeight: 25

                            wrapMode: Text.Wrap
                            text: "Your PIN is:"
                            font.pointSize: 8
                            font.kerning: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item{
                            Layout.alignment: Qt.AlignHCenter

                            Layout.maximumWidth: 20
                            Layout.preferredWidth: 20
                            Layout.minimumWidth: 20

                            Layout.maximumHeight: 20
                            Layout.preferredHeight: 20
                            Layout.minimumHeight: 20
                        }

                        Label{
                            id: exportedPIN

                            Layout.alignment: Qt.AlignHCenter

                            Layout.preferredHeight: 25

                            wrapMode: Text.Wrap
                            text: "PIN"
                            font.pointSize: 12
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

                        Label{
                            id: infoLabel

                            property bool isSucessState: false
                            property int borderWidth : isSucessState? 1 : 0
                            property int borderRadius : isSucessState? 15 : 0
                            property string backgroundColor : isSucessState? "whitesmoke" : "transparent"
                            property string borderColor : isSucessState? "lightgray" : "transparent"
                            color: isSucessState ? "#2b5084" : "black"
                            padding: isSucessState ? 8 : 0

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 320
                            Layout.preferredHeight: 50

                            wrapMode: Text.Wrap
                            text: qsTr("This pin and the account password should be entered in your device within 10 minutes.")
                            font.pointSize: 8
                            font.kerning: true

                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter

                            background: Rectangle{
                                id: infoLabelBackground

                                anchors.fill: parent
                                border.width: infoLabel.borderWidth
                                border.color: infoLabel.borderColor
                                radius: infoLabel.borderRadius
                                color: infoLabel.backgroundColor
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

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
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
                            id: btnCloseExportDialog

                            Layout.alignment: Qt.AlignHCenter

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
                                if(infoLabel.isSucessState){
                                    accept()
                                } else {
                                    reject()
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
                        Layout.fillHeight: true

                        Layout.maximumWidth: 20
                        Layout.preferredWidth: 20
                        Layout.minimumWidth: 20
                    }
                }
            }
        }
    }
}
