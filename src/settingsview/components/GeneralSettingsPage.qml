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
import net.jami.Adapters 1.0
import net.jami.Enums 1.0
import "../../commoncomponents"

Rectangle {
    id: root

    function populateGeneralSettings(){
        // settings
        closeOrMinCheckBox.checked = SettingsAdapter.getAppValue(Settings.MinimizeOnClose)
        applicationOnStartUpCheckBox.checked = UtilsAdapter.checkStartupLink()
        notificationCheckBox.checked = SettingsAdapter.getAppValue(Settings.EnableNotifications)

        alwaysRecordingCheckBox.checked = ClientWrapper.avmodel.getAlwaysRecord()
        recordPreviewCheckBox.checked = ClientWrapper.avmodel.getRecordPreview()
        recordQualityValueLabel.text = UtilsAdapter.getRecordQualityString(ClientWrapper.avmodel.getRecordQuality() / 100)
        recordQualitySlider.value = ClientWrapper.avmodel.getRecordQuality() / 100

        ClientWrapper.avmodel.setRecordPath(ClientWrapper.SettingsAdapter.getDir_Document())

        autoUpdateCheckBox.checked = SettingsAdapter.getAppValue(Settings.Key.AutoUpdate)
    }

    function setEnableNotifications(state) {
        SettingsAdapter.setAppValue(Settings.Key.EnableNotifications, state)
    }

    function setMinimizeOnClose(state) {
        SettingsAdapter.setAppValue(Settings.Key.MinimizeOnClose, state)
    }

    function slotSetRunOnStartUp(state){
        SettingsAdapter.setRunOnStartUp(state)
    }

    function setAutoUpdate(state) {
        SettingsAdapter.setAppValue(Settings.Key.AutoUpdate, state)
    }

    function slotAlwaysRecordingClicked(state){
        ClientWrapper.avmodel.setAlwaysRecord(state)
    }

    function slotRecordPreviewClicked(state){
        ClientWrapper.avmodel.setRecordPreview(state)
    }

    function slotRecordQualitySliderValueChanged(value){
        recordQualityValueLabel.text = UtilsAdapter.getRecordQualityString(value)
        updateRecordQualityTimer.restart()
    }

    Timer{
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
            var dir = UtilsAdapter.getAbsPath(folder.toString())
            downloadPath = dir
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
            var dir = UtilsAdapter.getAbsPath(folder.toString())
            recordPath = dir
        }
    }

    //TODO: complete check for update and check for Beta slot functions
    function checkForUpdateSlot(){}
    function installBetaSlot(){}

    // settings
    property string downloadPath: SettingsAdapter.getDir_Download()

    // recording
    property string recordPath: SettingsAdapter.getDir_Document()

    property int preferredColumnWidth : root.width / 2 - 50
    property int preferredSettingsWidth : root.width - 100

    signal backArrowClicked

    onDownloadPathChanged: {
        if(downloadPath === "") return
       SettingsAdapter.setDownloadPath(downloadPath)
    }

    onRecordPathChanged: {
        if(recordPath === "") return

        if(ClientWrapper.avmodel){
            ClientWrapper.avmodel.setRecordPath(recordPath)
        }
    }

    ColumnLayout {
        anchors.fill: root

        RowLayout {
            id: generalSettingsTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.fillWidth: true
            Layout.preferredHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                radius: JamiTheme.preferredFieldHeight
                source: "qrc:/images/icons/ic_arrow_back_24px.svg"
                backgroundColor: "white"
                onExitColor: "white"
                toolTipText: qsTr("Toggle to display side panel")
                hoverEnabled: true
                visible: mainViewWindow.sidePanelHidden

                onClicked: {
                    backArrowClicked()
                }
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("General")
                font.pointSize: JamiTheme.titleFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

        }

        ScrollView {
            id: generalSettingsScrollView

            Layout.fillHeight: true
            Layout.fillWidth: true

            focus: true
            clip: true

            ColumnLayout {
                width: root.width

                // system setting panel
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    Label {
                        Layout.fillWidth: true

                        text: qsTr("System")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    ToggleSwitch {
                        id: notificationCheckBox
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        labelText: qsTr("Enable desktop notifications")
                        fontPointSize: JamiTheme.settingsFontSize

                        tooltipText: qsTr("toggle enable notifications")

                        onSwitchToggled: {
                            setEnableNotifications(checked)
                        }
                    }

                    ToggleSwitch {
                        id: closeOrMinCheckBox
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        labelText: qsTr("Keep minimize on close")
                        fontPointSize: JamiTheme.settingsFontSize

                        tooltipText: qsTr("toggle keep minimized on close")

                        onSwitchToggled: {
                            setMinimizeOnClose(checked)
                        }
                    }

                    ToggleSwitch {
                        id: applicationOnStartUpCheckBox
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        labelText: qsTr("Run On Startup")
                        fontPointSize: JamiTheme.settingsFontSize

                        tooltipText: qsTr("toggle run application on system startup")

                        onSwitchToggled: {
                            slotSetRunOnStartUp(checked)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: qsTr("Downloads folder")
                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        MaterialButton {
                            id: downloadButton

                            Layout.alignment: Qt.AlignRight
                            Layout.preferredWidth: preferredColumnWidth
                            Layout.fillHeight: true

                            toolTipText: qsTr("Press to choose download folder path")
                            text: downloadPath
                            source: "qrc:/images/icons/round-folder-24px.svg"
                            color: JamiTheme.buttonTintedGrey
                            hoveredColor: JamiTheme.buttonTintedGreyHovered
                            pressedColor: JamiTheme.buttonTintedGreyPressed

                            onClicked: {
                                openDownloadFolderSlot()
                            }
                        }
                    }
                }

                // // call recording setting panel
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true

                        eText: qsTr("Call Recording")
                        font.pointSize: JamiTheme.headerFontSize
                        maxWidth: width
                    }

                    ToggleSwitch {
                        id: alwaysRecordingCheckBox
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        labelText: qsTr("Always record calls")
                        fontPointSize: JamiTheme.settingsFontSize

                        onSwitchToggled: {
                            slotAlwaysRecordingClicked(checked)
                        }
                    }

                    ToggleSwitch {
                        id: recordPreviewCheckBox
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        labelText: qsTr("Record preview video for a call")
                        fontPointSize: JamiTheme.settingsFontSize

                        onSwitchToggled: {
                            slotRecordPreviewClicked(checked)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Text {
                            Layout.fillWidth: true
                            Layout.rightMargin: JamiTheme.preferredMarginSize / 2

                            text: qsTr("Quality")
                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            id: recordQualityValueLabel

                            Layout.alignment: Qt.AlignRight
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.rightMargin: JamiTheme.preferredMarginSize / 2

                            text: qsTr("VALUE ")

                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                        }

                        Slider {
                            id: recordQualitySlider

                            Layout.maximumWidth: preferredColumnWidth
                            Layout.alignment: Qt.AlignRight
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            from: 0
                            to: 500
                            stepSize: 1

                            onMoved: {
                                slotRecordQualitySliderValueChanged(value)
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: qsTr("Save in")
                            font.pointSize: JamiTheme.settingsFontSize
                            font.kerning: true

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        MaterialButton {
                            id: recordPathButton

                            Layout.alignment: Qt.AlignRight
                            Layout.fillHeight: true
                            Layout.preferredWidth: preferredColumnWidth

                            toolTipText: qsTr("Press to choose record folder path")
                            text: recordPath
                            source: "qrc:/images/icons/round-folder-24px.svg"
                            color: JamiTheme.buttonTintedGrey
                            hoveredColor: JamiTheme.buttonTintedGreyHovered
                            pressedColor: JamiTheme.buttonTintedGreyPressed

                            onClicked: {
                                openRecordFolderSlot()
                            }
                        }
                    }
                }

                // update setting panel
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize
                    visible: Qt.platform.os == "windows"? true : false

                    Label {
                        Layout.fillWidth: true
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("Updates")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    ToggleSwitch {
                        id: autoUpdateCheckBox

                        labelText: qsTr("Check for updates automatically")
                        fontPointSize: JamiTheme.settingsFontSize

                        tooltipText: qsTr("toggle automatic updates")

                        onSwitchToggled: {
                            setAutoUpdate(checked)
                        }
                    }

                    HoverableRadiusButton {
                        id: checkUpdateButton

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        radius: height / 2

                        toolTipText: qsTr("Check for updates now")
                        text: qsTr("Updates")
                        fontPointSize: JamiTheme.buttonFontSize

                        onClicked: {
                            checkForUpdateSlot()
                        }
                    }

                    HoverableRadiusButton {
                        id: installBetaButton

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: JamiTheme.preferredFieldWidth
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight

                        radius: height / 2

                        toolTipText: qsTr("Install the latest beta version")
                        text: qsTr("Beta Install")
                        fontPointSize: JamiTheme.buttonFontSize

                        onClicked: {
                            installBetaSlot()
                        }
                    }
                }
            }
        }
    }
}
