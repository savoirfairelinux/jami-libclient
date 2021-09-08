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

    signal accepted

    function openLinkDeviceDialog() {
        infoLabel.text = JamiStrings.pinTimerInfos
        passwordEdit.clear()

        open()

        if(AccountAdapter.hasPassword()) {
            stackedWidget.currentIndex = enterPasswordPage.pageIndex
            passwordEdit.forceActiveFocus()
        } else {
            setGeneratingPage()
        }
    }

    function setGeneratingPage() {
        if(passwordEdit.length === 0 && AccountAdapter.hasPassword()){
            setExportPage(NameDirectory.ExportOnRingStatus.WRONG_PASSWORD, "")
            return
        }

        stackedWidget.currentIndex = exportingSpinnerPage.pageIndex
        spinnerMovie.playing = true

        timerForExport.restart()
    }

    function setExportPage(status, pin) {
        if (status === NameDirectory.ExportOnRingStatus.SUCCESS) {
            infoLabel.success = true
            infoLabelsRowLayout.visible = true
            infoLabel.text = JamiStrings.pinTimerInfos
            exportedPIN.text = pin
        } else {
            infoLabel.success = false
            infoLabelsRowLayout.visible = false

            switch(status) {
            case NameDirectory.ExportOnRingStatus.WRONG_PASSWORD:
                infoLabel.text = JamiStrings.incorrectPassword
                break
            case NameDirectory.ExportOnRingStatus.NETWORK_ERROR:
                infoLabel.text = JamiStrings.linkDeviceNetWorkError
                break
            case NameDirectory.ExportOnRingStatus.INVALID:
                infoLabel.text = JamiStrings.somethingWentWrong
                break
            }
        }
        stackedWidget.currentIndex = exportingInfoPage.pageIndex
    }

    title: JamiStrings.addDevice

    Timer{
        id: timerForExport

        repeat: false
        interval: 200

        onTriggered: {
            AccountAdapter.model.exportOnRing(LRCInstance.currentAccountId,
                                              passwordEdit.text)
        }
    }

    Connections {
        target: NameDirectory

        function onExportOnRingEnded(status, pin) {
            setExportPage(status, pin)
        }
    }

    contentItem: Rectangle {
        id: linkDeviceContentRect

        color: JamiTheme.secondaryBackgroundColor
        implicitWidth: JamiTheme.preferredDialogWidth
        implicitHeight: JamiTheme.preferredDialogHeight

        StackLayout {
            id: stackedWidget

            anchors.centerIn: parent
            anchors.fill: parent
            anchors.margins: JamiTheme.preferredMarginSize

            // Index = 0
            Item {
                id: enterPasswordPage

                readonly property int pageIndex: 0

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 16

                    Label {
                        Layout.alignment: Qt.AlignHCenter

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
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter

                        placeholderText: JamiStrings.enterCurrentPassword

                        onTextChanged: {
                            btnConfirm.enabled = text.length > 0
                        }

                        onAccepted: btnConfirm.clicked()
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        Layout.fillWidth: true
                        spacing: 16

                        MaterialButton {
                            id: btnConfirm

                            Layout.alignment: Qt.AlignHCenter

                            preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                            preferredHeight: JamiTheme.preferredFieldHeight

                            color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                            hoveredColor: JamiTheme.buttonTintedBlackHovered
                            pressedColor: JamiTheme.buttonTintedBlackPressed
                            outlined: true
                            enabled: false

                            text: JamiStrings.exportAccount

                            onClicked: setGeneratingPage()
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
                            enabled: true

                            text: JamiStrings.optionCancel

                            onClicked: close()
                        }
                    }
                }
            }

            // Index = 1
            Item {
                id: exportingSpinnerPage

                readonly property int pageIndex: 1

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 16

                    Label {
                        Layout.alignment: Qt.AlignCenter

                        text: JamiStrings.backupAccount
                        color: JamiTheme.textColor
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true
                        horizontalAlignment: Text.AlignLeft
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
                id: exportingInfoPage

                readonly property int pageIndex: 2

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 16

                    RowLayout {
                        id: infoLabelsRowLayout

                        Layout.alignment: Qt.AlignCenter
                        Layout.fillWidth: true
                        Layout.margins: JamiTheme.preferredMarginSize

                        spacing: 16

                        Label {
                            id: yourPinLabel

                            Layout.alignment: Qt.AlignHCenter

                            text: JamiStrings.yourPinIs
                            color: JamiTheme.textColor
                            font.pointSize: JamiTheme.headerFontSize
                            font.kerning: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        MaterialLineEdit {
                            id: exportedPIN

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: fieldLayoutWidth

                            text: JamiStrings.pin
                            color: JamiTheme.textColor
                            selectByMouse: true
                            readOnly: true
                            font.pointSize: JamiTheme.headerFontSize
                            font.kerning: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Label {
                        id: infoLabel

                        property bool success: false
                        property int borderWidth : success? 1 : 0
                        property int borderRadius : success? 15 : 0
                        property string backgroundColor : success? "whitesmoke" : "transparent"
                        property string borderColor : success? "lightgray" : "transparent"

                        Layout.maximumWidth: linkDeviceContentRect.width -
                                             JamiTheme.preferredMarginSize * 2

                        Layout.alignment: Qt.AlignCenter

                        color: success ? JamiTheme.successLabelColor : JamiTheme.redColor
                        padding: success ? 8 : 0

                        wrapMode: Text.Wrap
                        text: JamiStrings.pinTimerInfos
                        font.pointSize: JamiTheme.textFontSize
                        font.kerning: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        background: Rectangle {
                            id: infoLabelBackground

                            border.width: infoLabel.borderWidth
                            border.color: infoLabel.borderColor
                            radius: infoLabel.borderRadius
                            color: JamiTheme.secondaryBackgroundColor
                        }
                    }

                    MaterialButton {
                        id: btnCloseExportDialog

                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

                        preferredWidth: JamiTheme.preferredFieldWidth / 2 - 8
                        preferredHeight: JamiTheme.preferredFieldHeight

                        color: enabled ? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
                        hoveredColor: JamiTheme.buttonTintedBlackHovered
                        pressedColor: JamiTheme.buttonTintedBlackPressed
                        outlined: true
                        enabled: true

                        text: JamiStrings.close

                        onClicked: {
                            if (infoLabel.success)
                                accepted()
                            close()
                        }
                    }
                }
            }
        }
    }
}

