/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import "../../commoncomponents"

Rectangle {
    id: generalSettingsRect

    function populateGeneralSettings(){
        // settings
        closeOrMinCheckBox.checked = ClientWrapper.settingsAdaptor.getSettingsValue_CloseOrMinimized()
        applicationOnStartUpCheckBox.checked = ClientWrapper.utilsAdaptor.checkStartupLink()
        notificationCheckBox.checked = ClientWrapper.settingsAdaptor.getSettingsValue_EnableNotifications()

        alwaysRecordingCheckBox.checked = ClientWrapper.avmodel.getAlwaysRecord()
        recordPreviewCheckBox.checked = ClientWrapper.avmodel.getRecordPreview()
        recordQualityValueLabel.text = ClientWrapper.utilsAdaptor.getRecordQualityString(ClientWrapper.avmodel.getRecordQuality() / 100)
        recordQualitySlider.value = ClientWrapper.avmodel.getRecordQuality() / 100

        ClientWrapper.avmodel.setRecordPath(ClientWrapper.settingsAdaptor.getDir_Document())

        autoUpdateCheckBox.checked = ClientWrapper.settingsAdaptor.getSettingsValue_AutoUpdate()
    }

    function slotSetNotifications(state){
        ClientWrapper.settingsAdaptor.setNotifications(state)
    }

    function slotSetClosedOrMin(state){
        ClientWrapper.settingsAdaptor.setClosedOrMin(state)
    }

    function slotSetRunOnStartUp(state){
        ClientWrapper.settingsAdaptor.setRunOnStartUp(state)
    }

    function slotSetUpdateAutomatic(state){
        ClientWrapper.settingsAdaptor.setUpdateAutomatic(state)
    }

    function slotAlwaysRecordingClicked(state){
        ClientWrapper.avmodel.setAlwaysRecord(state)
    }

    function slotRecordPreviewClicked(state){
        ClientWrapper.avmodel.setRecordPreview(state)
    }

    function slotRecordQualitySliderValueChanged(value){
        recordQualityValueLabel.text = ClientWrapper.utilsAdaptor.getRecordQualityString(value)
        updateRecordQualityTimer.restart()
    }

    Timer {
        id: updateRecordQualityTimer

        interval: 500

        onTriggered: {
            slotRecordQualitySliderSliderReleased()
        }
    }

    function slotRecordQualitySliderSliderReleased(){
        var value = recordQualitySlider.value
        ClientWrapper.avmodel.setRecordQuality(value * 100)
    }

    function openDownloadFolderSlot(){
        downloadPathDialog.open()
    }

    FolderDialog {
        id: downloadPathDialog

        title: qsTr("Select A Folder For Your Downloads")
        currentFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

        onAccepted: {
            var dir = ClientWrapper.utilsAdaptor.getAbsPath(folder.toString())
            downloadPath = dir
        }

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }
    }

    function openRecordFolderSlot(){
        recordPathDialog.open()
    }

    FolderDialog {
        id: recordPathDialog

        title: qsTr("Select A Folder For Your Recordings")
        currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)

        onAccepted: {
            var dir = ClientWrapper.utilsAdaptor.getAbsPath(folder.toString())
            recordPath = dir
        }

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }
    }

    //TODO: complete check for update and check for Beta slot functions
    function checkForUpdateSlot(){}
    function installBetaSlot(){}

    // settings
    property string downloadPath: ClientWrapper.settingsAdaptor.getDir_Download()

    // recording
    //property AVModel avmodel: ClientWrapper.accountAdaptor.avModel()
    property string recordPath: ClientWrapper.settingsAdaptor.getDir_Document()

    signal backArrowClicked

    onDownloadPathChanged: {
        if(downloadPath === "") return
        ClientWrapper.settingsAdaptor.setDownloadPath(downloadPath)
    }

    onRecordPathChanged: {
        if(recordPath === "") return

        if(ClientWrapper.avmodel){
            ClientWrapper.avmodel.setRecordPath(recordPath)
        }
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            spacing: 8

            Layout.fillHeight: true
            Layout.maximumWidth: 580
            Layout.preferredWidth: 580
            Layout.minimumWidth: 580

            RowLayout {

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.leftMargin: 16
                Layout.fillWidth: true
                Layout.maximumHeight: 64
                Layout.minimumHeight: 64
                Layout.preferredHeight: 64

                HoverableButton {
                    id: backToSettingsMenuButton

                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30

                    radius: 30
                    source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                    backgroundColor: "white"
                    onExitColor: "white"

                    visible: mainViewWindow.sidePanelHidden

                    onClicked: {
                        backArrowClicked()
                    }
                }

                Label {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 64
                    Layout.preferredHeight: 64
                    Layout.maximumHeight: 64

                    text: qsTr("General")
                    font.pointSize: JamiTheme.titleFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // system setting panel
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 21
                    Layout.preferredHeight: 21
                    Layout.maximumHeight: 21

                    text: qsTr("System")
                    font.pointSize: 13
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 10
                    Layout.preferredHeight: 10
                    Layout.maximumHeight: 10
                }

                ColumnLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    ToggleSwitch {
                        id: notificationCheckBox

                        Layout.leftMargin: 20

                        labelText: "Enable desktop notifications"
                        fontPointSize: 11

                        onSwitchToggled: {
                            slotSetNotifications(checked)
                        }
                    }

                    ToggleSwitch {
                        id: closeOrMinCheckBox

                        Layout.leftMargin: 20

                        labelText: "Keep minimize on close"
                        fontPointSize: 11

                        onSwitchToggled: {
                            slotSetClosedOrMin(checked)
                        }
                    }

                    ToggleSwitch {
                        id: applicationOnStartUpCheckBox

                        Layout.leftMargin: 20

                        labelText: "Run on Startup"
                        fontPointSize: 11

                        onSwitchToggled: {
                            slotSetRunOnStartUp(checked)
                        }
                    }

                    RowLayout {
                        spacing: 6

                        Layout.leftMargin: 20
                        Layout.fillWidth: true
                        Layout.maximumHeight: 30

                        Label {
                            Layout.fillHeight: true

                            Layout.maximumWidth: 94
                            Layout.preferredWidth: 94
                            Layout.minimumWidth: 94

                            text: qsTr("Download folder")
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }

                        HoverableRadiusButton {
                            id: downloadButton

                            Layout.maximumWidth: 320
                            Layout.preferredWidth: 320
                            Layout.minimumWidth: 320

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            radius: height / 2

                            icon.source: "qrc:/images/icons/round-folder-24px.svg"
                            icon.height: 24
                            icon.width: 24

                            text: downloadPath
                            fontPointSize: 10

                            onClicked: {
                                openDownloadFolderSlot()
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumHeight: 20
                Layout.preferredHeight: 20
                Layout.maximumHeight: 20
            }

            // call recording setting panel
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 21
                    Layout.preferredHeight: 21
                    Layout.maximumHeight: 21

                    text: qsTr("Call Recording")
                    font.pointSize: 13
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 10
                    Layout.preferredHeight: 10
                    Layout.maximumHeight: 10
                }

                ColumnLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    ToggleSwitch {
                        id: alwaysRecordingCheckBox

                        Layout.leftMargin: 20

                        labelText: "Always record calls"
                        fontPointSize: 11

                        onSwitchToggled: {
                            slotAlwaysRecordingClicked(checked)
                        }
                    }

                    ToggleSwitch {
                        id: recordPreviewCheckBox

                        Layout.leftMargin: 20

                        labelText: "Record preview video for a call"
                        fontPointSize: 11

                        onSwitchToggled: {
                            slotRecordPreviewClicked(checked)
                        }
                    }

                    RowLayout {
                        spacing: 6
                        Layout.leftMargin: 20
                        Layout.fillWidth: true
                        Layout.maximumHeight: 30

                        Label {
                            Layout.fillHeight: true

                            Layout.maximumWidth: 42
                            Layout.preferredWidth: 42
                            Layout.minimumWidth: 42

                            text: qsTr("Quality")
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }

                        ColumnLayout {
                            spacing: 0
                            Layout.fillHeight: true

                            Layout.maximumWidth: recordQualityValueLabel.width
                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }

                            Label {
                                id: recordQualityValueLabel

                                Layout.minimumWidth: 40

                                Layout.minimumHeight: 16
                                Layout.preferredHeight: 16
                                Layout.maximumHeight: 16

                                text: qsTr("VALUE ")

                                font.pointSize: 10
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                        }

                        Slider {
                            id: recordQualitySlider

                            Layout.fillHeight: true

                            Layout.maximumWidth: 320
                            Layout.preferredWidth: 320
                            Layout.minimumWidth: 320

                            from: 0
                            to: 500
                            stepSize: 1

                            onMoved: {
                                slotRecordQualitySliderValueChanged(value)
                            }
                        }
                    }

                    RowLayout {
                        spacing: 6

                        Layout.leftMargin: 20
                        Layout.fillWidth: true
                        Layout.maximumHeight: 30

                        Label {
                            Layout.fillHeight: true

                            Layout.maximumWidth: 42
                            Layout.preferredWidth: 42
                            Layout.minimumWidth: 42

                            text: qsTr("Save in")
                            font.pointSize: 10
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }

                        HoverableRadiusButton {
                            id: recordPathButton

                            Layout.maximumWidth: 320
                            Layout.preferredWidth: 320
                            Layout.minimumWidth: 320

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            radius: height / 2

                            icon.source: "qrc:/images/icons/round-folder-24px.svg"
                            icon.height: 24
                            icon.width: 24

                            text: recordPath
                            fontPointSize: 10

                            onClicked: {
                                openRecordFolderSlot()
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumHeight: 20
                Layout.preferredHeight: 20
                Layout.maximumHeight: 20
            }

            // update setting panel
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true
                visible: Qt.platform.os == "windows"? true : false

                Label {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 21
                    Layout.preferredHeight: 21
                    Layout.maximumHeight: 21

                    text: qsTr("Updates")
                    font.pointSize: 13
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true

                    Layout.minimumHeight: 10
                    Layout.preferredHeight: 10
                    Layout.maximumHeight: 10
                }

                ColumnLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    ToggleSwitch {
                        id: autoUpdateCheckBox

                        Layout.leftMargin: 20

                        labelText: "Check for updates automatically"
                        fontPointSize: 11

                        onSwitchToggled: {
                            slotSetUpdateAutomatic(checked)
                        }
                    }

                    RowLayout {
                        spacing: 6

                        Layout.leftMargin: 20
                        Layout.fillWidth: true
                        Layout.maximumHeight: 30

                        HoverableRadiusButton {
                            id: checkUpdateButton

                            Layout.maximumWidth: 275
                            Layout.preferredWidth: 275
                            Layout.minimumWidth: 275

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            radius: height / 2

                            text: "Check for updates now"
                            fontPointSize: 10

                            onClicked: {
                                checkForUpdateSlot()
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        spacing: 6

                        Layout.leftMargin: 20
                        Layout.fillWidth: true
                        Layout.maximumHeight: 30

                        HoverableRadiusButton {
                            id: installBetaButton

                            Layout.maximumWidth: 275
                            Layout.preferredWidth: 275
                            Layout.minimumWidth: 275

                            Layout.minimumHeight: 30
                            Layout.preferredHeight: 30
                            Layout.maximumHeight: 30

                            radius: height / 2

                            text: "Install the latest beta version"
                            fontPointSize: 10

                            onClicked: {
                                installBetaSlot()
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // spacer on the bottom
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}
