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
    id: generalSettingsRect

    function populateGeneralSettings(){
        // settings
        closeOrMinCheckBox.checked = SettingsAdapter.getAppValue(Settings.MinimizeOnClose)
        applicationOnStartUpCheckBox.checked = ClientWrapper.utilsAdaptor.checkStartupLink()
        notificationCheckBox.checked = SettingsAdapter.getAppValue(Settings.EnableNotifications)

        alwaysRecordingCheckBox.checked = ClientWrapper.avmodel.getAlwaysRecord()
        recordPreviewCheckBox.checked = ClientWrapper.avmodel.getRecordPreview()
        recordQualityValueLabel.text = ClientWrapper.utilsAdaptor.getRecordQualityString(ClientWrapper.avmodel.getRecordQuality() / 100)
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
        recordQualityValueLabel.text = ClientWrapper.utilsAdaptor.getRecordQualityString(value)
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
    property string downloadPath: SettingsAdapter.getDir_Download()

    // recording
    property string recordPath: SettingsAdapter.getDir_Document()

    property int preferredColumnWidth : generalSettingsScrollView.width / 2 - 50
    property int preferredSettingsWidth : generalSettingsScrollView.width - 100

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

    Layout.fillHeight: true
    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView
    anchors.centerIn: parent

    ColumnLayout {
        anchors.fill: generalSettingsRect

        RowLayout {
            id: generalSettingsTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.fillWidth: true
            Layout.maximumHeight: 64
            Layout.minimumHeight: 64
            Layout.preferredHeight: 64

            HoverableButton {
                id: backToSettingsMenuButton

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.preferredWidth: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.rightMargin: JamiTheme.preferredMarginSize

                radius: 32
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
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                text: qsTr("General")
                font.pointSize: JamiTheme.titleFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

        }

        ScrollView{
            id: generalSettingsScrollView
            property ScrollBar vScrollBar: ScrollBar.vertical

            Layout.fillHeight: true
            Layout.fillWidth: true

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            width: generalSettingsRect.width
            height: generalSettingsRect.height - generalSettingsTitle.height

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            clip: true

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                spacing: 24

                // system setting panel
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    Label {
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("System")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    ColumnLayout {
                        spacing: 8
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ToggleSwitch {
                            id: notificationCheckBox

                            labelText: desktopNotificationsElidedText.elidedText
                            fontPointSize: JamiTheme.settingsFontSize

                            tooltipText: qsTr("toggle enable notifications")

                            onSwitchToggled: {
                                setEnableNotifications(checked)
                            }
                        }

                        TextMetrics {
                            id: desktopNotificationsElidedText
                            elide: Text.ElideRight
                            elideWidth: preferredSettingsWidth
                            text:  qsTr("Enable desktop notifications")
                        }


                        ToggleSwitch {
                            id: closeOrMinCheckBox

                            labelText: keepMinimizeElidedText.elidedText
                            fontPointSize: JamiTheme.settingsFontSize

                            tooltipText: qsTr("toggle keep minimized on close")

                            onSwitchToggled: {
                                setMinimizeOnClose(checked)
                            }
                        }

                        TextMetrics {
                            id: keepMinimizeElidedText
                            elide: Text.ElideRight
                            elideWidth: preferredSettingsWidth
                            text:  qsTr("Keep minimize on close")
                        }


                        ToggleSwitch {
                            id: applicationOnStartUpCheckBox

                            labelText: runOnStartupElidedText.elidedText
                            fontPointSize: JamiTheme.settingsFontSize

                            tooltipText: qsTr("toggle run application on system startup")

                            onSwitchToggled: {
                                slotSetRunOnStartUp(checked)
                            }
                        }

                        TextMetrics {
                            id: runOnStartupElidedText
                            elide: Text.ElideRight
                            elideWidth: preferredSettingsWidth
                            text:  qsTr("Run On Startup")
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            ElidedTextLabel {

                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                                eText: qsTr("Downloads folder")
                                font.pointSize: JamiTheme.settingsFontSize
                                maxWidth: preferredColumnWidth
                            }

                            MaterialButton {
                                id: downloadButton

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth

                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                Layout.alignment: Qt.AlignRight

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
                }

                // call recording setting panel
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize

                    ElidedTextLabel {
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        eText: qsTr("Call Recording")
                        font.pointSize: JamiTheme.headerFontSize
                        maxWidth: preferredSettingsWidth
                    }

                    ColumnLayout {
                        spacing: 8
                        Layout.fillWidth: true
                        Layout.leftMargin: JamiTheme.preferredMarginSize

                        ToggleSwitch {
                            id: alwaysRecordingCheckBox

                            labelText: alwaysRecordElidedText.elidedText
                            fontPointSize: JamiTheme.settingsFontSize

                            onSwitchToggled: {
                                slotAlwaysRecordingClicked(checked)
                            }
                        }

                        TextMetrics {
                            id: alwaysRecordElidedText
                            elide: Text.ElideRight
                            elideWidth: preferredSettingsWidth
                            text:  qsTr("Always record calls")
                        }


                        ToggleSwitch {
                            id: recordPreviewCheckBox

                            labelText: recordPreviewElidedText.elidedText
                            fontPointSize: JamiTheme.settingsFontSize

                            onSwitchToggled: {
                                slotRecordPreviewClicked(checked)
                            }
                        }

                        TextMetrics {
                            id: recordPreviewElidedText
                            elide: Text.ElideRight
                            elideWidth: preferredSettingsWidth
                            text:  qsTr("Record preview video for a call")
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            Label {

                                Layout.fillWidth: true
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                                text: qsTr("Quality")
                                font.pointSize: JamiTheme.settingsFontSize
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Label {
                                id: recordQualityValueLabel

                                Layout.minimumWidth: 50
                                Layout.preferredWidth: 50
                                Layout.maximumWidth: 50

                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                text: qsTr("VALUE ")

                                font.pointSize: JamiTheme.settingsFontSize
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Slider{
                                id: recordQualitySlider

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth

                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                from: 0
                                to: 500
                                stepSize: 1

                                onMoved: {
                                    slotRecordQualitySliderValueChanged(value)
                                }
                            }
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            Label {
                                Layout.fillWidth: true
                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                text: qsTr("Save in")
                                font.pointSize: JamiTheme.settingsFontSize
                                font.kerning: true

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            MaterialButton {
                                id: recordPathButton

                                Layout.maximumWidth: preferredColumnWidth
                                Layout.preferredWidth: preferredColumnWidth
                                Layout.minimumWidth: preferredColumnWidth

                                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                                Layout.alignment: Qt.AlignRight

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
                }

                // update setting panel
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    visible: Qt.platform.os == "windows"? true : false

                    Label {
                        Layout.fillWidth: true
                        Layout.minimumHeight: JamiTheme.preferredFieldHeight
                        Layout.preferredHeight: JamiTheme.preferredFieldHeight
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        text: qsTr("Updates")
                        font.pointSize: JamiTheme.headerFontSize
                        font.kerning: true

                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    ColumnLayout {
                        spacing: 8
                        Layout.fillWidth: true

                        ToggleSwitch {
                            id: autoUpdateCheckBox

                            labelText: autoUpdateText.elidedText
                            fontPointSize: JamiTheme.settingsFontSize

                            tooltipText: qsTr("toggle automatic updates")

                            onSwitchToggled: {
                                setAutoUpdate(checked)
                            }
                        }

                        TextMetrics {
                            id: autoUpdateText
                            elide: Text.ElideRight
                            elideWidth: preferredSettingsWidth
                            text: qsTr("Check for updates automatically")
                        }

                        HoverableRadiusButton {
                            id: checkUpdateButton

                            Layout.alignment: Qt.AlignHCenter
                            Layout.maximumWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.minimumWidth: JamiTheme.preferredFieldWidth
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

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
                            Layout.maximumWidth: JamiTheme.preferredFieldWidth
                            Layout.preferredWidth: JamiTheme.preferredFieldWidth
                            Layout.minimumWidth: JamiTheme.preferredFieldWidth
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                Item {
                    Layout.preferredWidth: generalSettingsRect.width - 32
                    Layout.minimumWidth: generalSettingsRect.width - 32
                    Layout.maximumWidth: JamiTheme.maximumWidthSettingsView - 32
                    Layout.fillHeight: true
                }
            }
        }
    }
}
