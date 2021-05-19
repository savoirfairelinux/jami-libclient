/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Trevor Tabah <trevor.tabah@savoirfairelinux.com>
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
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import Qt.labs.platform 1.1
import QtQuick.Dialogs 1.2

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Dialog {
    id: root

    property bool cancelPressed: false
    property bool startedLogs: false
    property bool isStopped: false
    property int itemWidth: Math.min(root.width / 2 - 50, 350) * 1.5
    property int widthDivisor: 4
    property int selectBeginning
    property int selectEnd


    function findNthIndexInText(substring, n){
        var i;
        var t = logsText.text
        var index = t.indexOf(substring)
        for (i = 0; i < n - 1; i++){
            index = t.indexOf(substring, index + 1)
        }
        return index
    }

    function monitor(continuous){
        SettingsAdapter.monitor(continuous)
    }

    Connections{
        target: SettingsAdapter
        function onDebugMessageReceived(message){
            var initialPosition = scroll.position
            var oldContent = flickable.contentY
            if (!root.cancelPressed){
                logsText.append(message);
            }
            if (logsText.lineCount >= 10000){
                var index = findNthIndexInText("\n", 10)
                logsText.remove(0, index)
            }
            var approximateBottom = (1.0 - flickable.visibleArea.heightRatio);
            if (!isStopped){

                if (initialPosition < 0){
                    flickable.flick(0, -(100))
                }
                else if (initialPosition >= approximateBottom * .8){
                    flickable.contentY = flickable.contentHeight - flickable.height
                    flickable.flick(0, -(flickable.maximumFlickVelocity))
                }
                else{
                    flickable.contentY = oldContent
                }
            }
        }
    }

    onVisibleChanged: {
        logsText.clear()
        copiedToolTip.close()
        if (startStopToggle.checked){
            startStopToggle.checked = false
            startedLogs = false
        }
        root.cancelPressed = true
        monitor(false)
    }

    title: JamiStrings.logsViewTitle
    modality: Qt.NonModal
    width: 800
    height: 700
    standardButtons: StandardButton.NoButton

    ColumnLayout{

        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.centerIn: parent
        height: root.height
        width: root.width

        Rectangle{
            id: buttonRectangleBackground
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            color: JamiTheme.backgroundColor

            border.color: color
            border.width: 0
            height: JamiTheme.preferredFieldHeight*2

            RowLayout{
                id: buttons

                Layout.alignment: Qt.AlignTop| Qt.AlignHCenter
                anchors.centerIn: parent

                ToggleSwitch {
                    id: startStopToggle

                    Layout.fillWidth: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    checked: false
                    labelText: JamiStrings.logsViewDisplay
                    fontPointSize: JamiTheme.settingsFontSize

                    onSwitchToggled: {
                        startedLogs = !startedLogs
                        if (startedLogs){
                            isStopped = false
                            root.cancelPressed = false
                            monitor(true)
                        }
                        else{
                            isStopped = true
                            root.cancelPressed = true
                            monitor(false)
                        }
                    }
                }

                MaterialButton{
                    id: showStatsButton

                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: itemWidth/widthDivisor
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize
                    Layout.alignment: Qt.AlignHCenter

                    text: JamiStrings.logsViewShowStats
                    color: startedLogs ? JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedBlack
                    hoveredColor: startedLogs ? JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedBlackHovered
                    pressedColor: startedLogs ? JamiTheme.buttonTintedGreyInactive : JamiTheme.buttonTintedBlackPressed
                    outlined: true

                    onClicked:{
                        if (!startedLogs){
                            root.cancelPressed = false
                            monitor(false)
                        }
                    }
                }

                MaterialButton{
                    id: clearButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: itemWidth / widthDivisor
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize

                    outlined: true
                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    text: JamiStrings.logsViewClear

                    onClicked: {
                        logsText.clear()
                        startedLogs = false
                        startStopToggle.checked = false
                        root.cancelPressed = true
                        monitor(false)
                    }
                }

                MaterialButton{
                    id: copyButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: itemWidth/widthDivisor

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed

                    outlined: true
                    text: JamiStrings.logsViewCopy

                    onClicked:{
                        logsText.selectAll()
                        logsText.copy()
                        logsText.deselect()
                        copiedToolTip.open()
                    }

                    ToolTip {
                        id: copiedToolTip

                        height: JamiTheme.preferredFieldHeight
                        TextArea{
                        text: JamiStrings.logsViewCopied
                            color: JamiTheme.textColor
                        }


                        background: Rectangle{
                            color: JamiTheme.primaryBackgroundColor
                        }
                   }
                }

                MaterialButton{
                    id: reportButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: itemWidth/widthDivisor
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    text: JamiStrings.logsViewReport
                    outlined: true

                    onClicked: Qt.openUrlExternally("https://jami.net/bugs-and-improvements/")
                }
            }
        }

        Rectangle{
            id: flickableRectangleBackground
            property alias text: logsText.text

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            color:  JamiTheme.primaryBackgroundColor
            border.color: color
            border.width: 6
            height: root.height - buttonRectangleBackground.height


            Flickable {
                id: flickable

                Layout.fillWidth: true
                Layout.fillHeight: true
                anchors.fill: flickableRectangleBackground

                boundsBehavior: Flickable.StopAtBounds

                TextArea.flickable: TextArea {
                    id: logsText

                    readOnly: true
                    text: ""
                    color: JamiTheme.textColor
                    wrapMode: TextArea.Wrap
                    selectByMouse: true

                    MouseArea{
                        anchors.fill: logsText
                        acceptedButtons: Qt.RightButton
                        hoverEnabled: true

                        onClicked:{
                            selectBeginning = logsText.selectionStart
                            selectEnd = logsText.selectionEnd
                            rightClickMenu.open()
                            logsText.select(selectBeginning, selectEnd)
                        }

                        Menu{
                            id: rightClickMenu

                            MenuItem{
                                text: JamiStrings.logsViewCopy
                                onTriggered:{
                                    logsText.copy()
                                }
                            }
                        }
                    }
                }
                ScrollBar.vertical: ScrollBar {
                    id: scroll
                }
            }
        }
    }


}
