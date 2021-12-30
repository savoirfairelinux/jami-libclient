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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Window {
    id: root

    property bool cancelPressed: false
    property bool logging: false
    property bool isStopped: false
    property bool hasOpened: false

    property int itemWidth: Math.min(root.width / 2 - 50, 350) * 1.5
    property int widthDivisor: 4
    property int selectBeginning
    property int selectEnd

    property var lineSize: []
    property var lineCounter: 0

    function monitor(continuous) {
        UtilsAdapter.monitor(continuous)
    }

    title: JamiStrings.logsViewTitle
    width: 600
    height: 500

    Connections {
        target: UtilsAdapter

        function onDebugMessageReceived(message) {
            if (!root.visible) {
                return
            }
            var initialPosition = scrollView.ScrollBar.vertical.position
            lineCounter += 1
            lineSize.push(message.length)
            if (!root.cancelPressed) {
                logsText.append(message)
            }
            if (lineCounter >= 10000) {
                lineCounter -= 1
                logsText.remove(0, lineSize[0])
                lineSize.shift()
            }
            scrollView.ScrollBar.vertical.position = initialPosition
                    > (.8 * (1.0 - scrollView.ScrollBar.vertical.size)) ?
                        1.0 - scrollView.ScrollBar.vertical.size : initialPosition
        }
    }

    onVisibleChanged: {
        if (visible && startStopToggle.checked) {
            if (hasOpened && lineCounter == 0) {
                var logList = UtilsAdapter.logList
                logsText.append(logList.join('\n'))
                lineCounter = logList.length
                lineSize.push(lineCounter ? logList[0].length : 0)
            }
        } else {
            logsText.clear()
            copiedToolTip.close()
            lineCounter = 0
            lineSize = []
        }
        hasOpened = true
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        Rectangle {
            id: buttonRectangleBackground

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight * 2

            border.width: 0
            color: JamiTheme.backgroundColor
            radius: JamiTheme.modalPopupRadius

            RowLayout {
                id: buttons

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
                        logging = !logging
                        if (logging) {
                            isStopped = false
                            root.cancelPressed = false
                            monitor(true)
                        } else {
                            isStopped = true
                            root.cancelPressed = true
                            monitor(false)
                        }
                    }
                }

                MaterialButton {
                    id: clearButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize

                    preferredWidth: itemWidth / widthDivisor
                    preferredHeight: JamiTheme.preferredFieldHeight

                    outlined: true
                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    text: JamiStrings.logsViewClear

                    onClicked: {
                        logsText.clear()
                        logging = false
                        startStopToggle.checked = false
                        root.cancelPressed = true
                        UtilsAdapter.logList = []
                        monitor(false)
                    }
                }

                MaterialButton {
                    id: copyButton

                    Layout.alignment: Qt.AlignHCenter

                    preferredWidth: itemWidth / widthDivisor
                    preferredHeight: JamiTheme.preferredFieldHeight

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed

                    outlined: true
                    text: JamiStrings.logsViewCopy

                    onClicked: {
                        logsText.selectAll()
                        logsText.copy()
                        logsText.deselect()
                        copiedToolTip.open()
                    }

                    ToolTip {
                        id: copiedToolTip

                        height: JamiTheme.preferredFieldHeight
                        TextArea {
                            text: JamiStrings.logsViewCopied
                            color: JamiTheme.textColor
                        }
                        background: Rectangle {
                            color: JamiTheme.primaryBackgroundColor
                        }
                    }
                }

                MaterialButton {
                    id: reportButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: JamiTheme.preferredMarginSize
                    Layout.bottomMargin: JamiTheme.preferredMarginSize
                    Layout.rightMargin: JamiTheme.preferredMarginSize

                    preferredWidth: itemWidth / widthDivisor
                    preferredHeight: JamiTheme.preferredFieldHeight

                    color: JamiTheme.buttonTintedBlack
                    hoveredColor: JamiTheme.buttonTintedBlackHovered
                    pressedColor: JamiTheme.buttonTintedBlackPressed
                    text: JamiStrings.logsViewReport
                    outlined: true

                    onClicked: Qt.openUrlExternally(
                                   "https://jami.net/bugs-and-improvements/")
                }
            }
        }

        JamiFlickable {
            id: scrollView

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            interactive: true
            attachedFlickableMoving: contentHeight > height || scrollView.moving

            TextArea.flickable: TextArea {
                id: logsText

                font.pointSize: JamiTheme.textFontSize
                font.hintingPreference: Font.PreferNoHinting

                readOnly: true
                color: JamiTheme.blackColor
                wrapMode: TextArea.Wrap
                selectByMouse: true

                background: Rectangle {
                    border.width: 0
                    color: JamiTheme.transparentColor
                }

                MouseArea {
                    anchors.fill: logsText
                    acceptedButtons: Qt.RightButton
                    hoverEnabled: true

                    onClicked: {
                        selectBeginning = logsText.selectionStart
                        selectEnd = logsText.selectionEnd
                        rightClickMenu.open()
                        logsText.select(selectBeginning, selectEnd)
                    }

                    Menu {
                        id: rightClickMenu

                        MenuItem {
                            text: JamiStrings.logsViewCopy
                            onTriggered: {
                                logsText.copy()
                            }
                        }
                    }
                }
            }
        }
    }
}
