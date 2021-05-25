/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.14

import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

BaseDialog {
    id: root

    visible: false
    title: qsTr("Shortcuts")

    contentItem: Rectangle {
        id: shortcutsTableContentRect

        implicitWidth: 800
        implicitHeight: 600
        color: JamiTheme.backgroundColor

        ListModel {
            id: keyboardGeneralShortcutsModel
            ListElement {
                Shortcut: "Ctrl+J"
                Description: qsTr("Open account list")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+L"
                Description: qsTr("Focus conversations list")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+R"
                Description: qsTr("Requests list")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+↑"
                Description: qsTr("Previous conversation")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+↓"
                Description: qsTr("Next conversation")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+F"
                Description: qsTr("Search bar")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "F11"
                Description: qsTr("Fullscreen")
                KeyLength: 1
            }
            // TODO: add the following after redesign
            // ListElement {
            //     Shortcut: Qt.platform.os !== "windows" ? "Ctrl+Q" : "Alt+F4"
            //     Description: Qt.platform.os !== "windows" ? qsTr("Quit") : qsTr("Exit")
            //     KeyLength: 2
            // }
        }
        ListModel {
            id: keyboardConversationShortcutsModel
            ListElement {
                Shortcut: "Shift+Ctrl+C"
                Description: qsTr("Start an audio call")
                KeyLength: 3
            }
            ListElement {
                Shortcut: "Shift+Ctrl+X"
                Description: qsTr("Start a video call")
                KeyLength: 3
            }
            ListElement {
                Shortcut: "Shift+Ctrl+L"
                Description: qsTr("Clear history")
                KeyLength: 3
            }
            ListElement {
                Shortcut: "Shift+Ctrl+B"
                Description: qsTr("Block contact")
                KeyLength: 3
            }
            ListElement {
                Shortcut: "Shift+Ctrl+A"
                Description: qsTr("Accept contact request")
                KeyLength: 3
            }
        }
        ListModel {
            id: keyboardSettingsShortcutsModel
            ListElement {
                Shortcut: "Ctrl+M"
                Description: qsTr("Media settings")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+G"
                Description: qsTr("General settings")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+I"
                Description: qsTr("Account settings")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+Shift+N"
                Description: qsTr("Open account creation wizard")
                KeyLength: 3
            }
            ListElement {
                Shortcut: "F10"
                Description: qsTr("Open window")
                KeyLength: 1
            }
        }
        ListModel {
            id: keyboardCallsShortcutsModel
            ListElement {
                Shortcut: "Ctrl+Y"
                Description: qsTr("Answer an incoming call")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+D"
                Description: qsTr("End call")
                KeyLength: 2
            }
            ListElement {
                Shortcut: "Ctrl+Shift+D"
                Description: qsTr("Decline the call request")
                KeyLength: 2
            }
        }
        Component {
            id: shortcutDelegateWithThreeKeys

            Rectangle {
                id: cellRectWithThreeKeys

                implicitWidth: JamiTheme.mainViewMinWidth / 2
                implicitHeight: 50
                anchors.left: parent.left
                anchors.leftMargin: 20
                color: JamiTheme.backgroundColor
                border.color: JamiTheme.backgroundColor

                Rectangle {
                    id: containerRectWithThreeKeys

                    implicitWidth: parent.width - 10
                    implicitHeight: 50

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: JamiTheme.backgroundColor

                    Component.onCompleted: {
                        var componentKeyOne = Qt.createComponent("KeyBoardShortcutKey.qml")
                        if (componentKeyOne.status === Component.Ready) {
                            var objectKeyOne = componentKeyOne.createObject(containerRectWithThreeKeys)
                            objectKeyOne.anchors.verticalCenter = containerRectWithThreeKeys.verticalCenter
                            objectKeyOne.anchors.left = containerRectWithThreeKeys.left
                            objectKeyOne.text = Qt.binding(function() { return modelData.Shortcut.split("+")[0] })
                        }
                        if (modelData.Shortcut.split("+").length === 1)
                            return
                        var componentPlusSign = Qt.createQmlObject('import QtQuick 2.0;' +
                                                                'Text {anchors.verticalCenter: containerRectWithThreeKeys.verticalCenter;' +
                                                                'anchors.verticalCenterOffset: -2;' +
                                                                'anchors.left: containerRectWithThreeKeys.left;' +
                                                                'anchors.leftMargin: 30;' +
                                                                'font.bold: true;' +
                                                                'font.pointSize : 12;' +
                                                                'text: "+"}',
                                                                containerRectWithThreeKeys)
                        componentPlusSign.color = Qt.binding(function() { return JamiTheme.textColor })
                        var componentKeyTwo = Qt.createComponent("KeyBoardShortcutKey.qml")
                        if (componentKeyTwo.status === Component.Ready) {
                            var objectKeyTwo = componentKeyTwo.createObject(containerRectWithThreeKeys)
                            objectKeyTwo.anchors.verticalCenter = containerRectWithThreeKeys.verticalCenter
                            objectKeyTwo.anchors.left = containerRectWithThreeKeys.left
                            objectKeyTwo.anchors.leftMargin = componentPlusSign.anchors.leftMargin + 42
                            objectKeyTwo.text = Qt.binding(function() { return modelData.Shortcut.split("+")[1] })
                        }

                        if (modelData.Shortcut.split("+").length === 2)
                            return
                        var componentPlusSignTwo = Qt.createQmlObject('import QtQuick 2.0;' +
                                                                    'Text {anchors.verticalCenter: containerRectWithThreeKeys.verticalCenter;' +
                                                                    'anchors.verticalCenterOffset: -2;' +
                                                                    'anchors.left: containerRectWithThreeKeys.left;' +
                                                                    'anchors.leftMargin: 97;' +
                                                                    'font.bold: true;' +
                                                                    'font.pointSize : 12;' +
                                                                    'text: "+"}',
                                                                    containerRectWithThreeKeys)
                        componentPlusSignTwo.color = Qt.binding(function() { return JamiTheme.textColor })
                        var componentKeyThree = Qt.createComponent("KeyBoardShortcutKey.qml")
                        if (componentKeyThree.status === Component.Ready) {
                            var objectKeyThree = componentKeyThree.createObject(containerRectWithThreeKeys)
                            objectKeyThree.anchors.verticalCenter = containerRectWithThreeKeys.verticalCenter
                            objectKeyThree.anchors.left = containerRectWithThreeKeys.left
                            objectKeyThree.anchors.leftMargin = componentPlusSignTwo.anchors.leftMargin + 35
                            objectKeyThree.text = Qt.binding(function() { return modelData.Shortcut.split("+")[2] })
                        }
                    }
                }
            }
        }
        Component {
            id: descriptionDelegate

            Rectangle {
                implicitWidth: root.width / 2
                implicitHeight: 50

                color: JamiTheme.backgroundColor
                border.color: JamiTheme.backgroundColor
                Text {
                    id : descriptionText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    font.family: "Arial"
                    font.pointSize: JamiTheme.textFontSize
                    text: styleData.value
                    color: JamiTheme.textColor
                }
            }
        }

        Column {
            spacing: 2
            id: columnAll
            anchors.rightMargin: 20
            anchors.leftMargin: 20
            anchors.bottomMargin: 20
            anchors.topMargin: 50

            width: JamiTheme.mainViewMinWidth
            height: JamiTheme.mainViewMinHeight - 100
            anchors.fill: parent

            StackLayout {
                // pages
                implicitWidth: parent.width
                implicitHeight: parent.height - tabBar.height
                currentIndex: tabBar.currentIndex
                Item {
                    id: tabOne
                    Rectangle {
                        implicitWidth: parent.width / 2
                        implicitHeight: parent.height
                        anchors.left: parent.left
                        TableView {
                            id: generalTableView
                            anchors.fill: parent
                            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                            frameVisible: false
                            TableViewColumn {
                                role: "Description"
                                width: generalTableView.width / 2
                                delegate: descriptionDelegate
                            }
                            TableViewColumn {
                                role: "Shortcut"
                                width: generalTableView.width / 2
                                delegate: Component{
                                    Loader {
                                        property variant modelData: model
                                        sourceComponent: shortcutDelegateWithThreeKeys
                                        asynchronous: true
                                    }
                                }
                            }
                            model: keyboardGeneralShortcutsModel
                            rowDelegate: Rectangle {
                                height: 50
                                color: JamiTheme.backgroundColor
                            }
                            style: TableViewStyle {
                                backgroundColor: JamiTheme.backgroundColor
                                alternateBackgroundColor: JamiTheme.backgroundColor
                                headerDelegate: Rectangle {
                                    // Only first column's header is shown
                                    height: [t_metrics_general.tightBoundingRect.height + 10, 0][styleData.column % 2]
                                    width: [parent.width, 0][styleData.column % 2]
                                    color: JamiTheme.backgroundColor
                                    radius: 4
                                    anchors.top: parent.top
                                    anchors.topMargin: 5
                                    Text {
                                        id : generalShortcutText
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        font.family: "Arial"
                                        font.pointSize: JamiTheme.headerFontSize
                                        text: styleData.column % 2 ? "" : "General"
                                        color: JamiTheme.textColor
                                    }
                                    TextMetrics {
                                        id:     t_metrics_general
                                        font:   generalShortcutText.font
                                        text:   generalShortcutText.text
                                    }
                                }
                            }
                        }
                    }
                    Rectangle {
                        implicitWidth: parent.width / 2
                        implicitHeight: parent.height
                        anchors.right: parent.right
                        TableView {
                            id: conversationsTableView
                            anchors.fill: parent
                            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                            frameVisible: false
                            TableViewColumn {
                                role: "Description"
                                width: conversationsTableView.width / 2
                                delegate: descriptionDelegate
                            }
                            TableViewColumn {
                                role: "Shortcut"
                                width: conversationsTableView.width / 2
                                delegate: Component{
                                    Loader {
                                        property variant modelData: model
                                        sourceComponent: shortcutDelegateWithThreeKeys
                                        asynchronous: true
                                    }
                                }
                            }
                            model: keyboardConversationShortcutsModel
                            rowDelegate: Rectangle {
                                height: 50
                                color: JamiTheme.backgroundColor
                            }
                            style: TableViewStyle {
                                backgroundColor: JamiTheme.backgroundColor
                                alternateBackgroundColor: JamiTheme.backgroundColor
                                headerDelegate: Rectangle {
                                    // Only first column's header is shown
                                    height: [t_metrics_conversations.tightBoundingRect.height + 10, 0][styleData.column % 2]
                                    width: [parent.width, 0][styleData.column % 2]
                                    color: JamiTheme.backgroundColor
                                    radius: 4
                                    anchors.top: parent.top
                                    anchors.topMargin: 5
                                    Text {
                                        id : conversationsShortcutText
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        font.family: "Arial"
                                        font.pointSize: JamiTheme.headerFontSize
                                        text: styleData.column % 2 ? "" : JamiStrings.conversations
                                        color: JamiTheme.textColor
                                    }
                                    TextMetrics {
                                        id:     t_metrics_conversations
                                        font:   conversationsShortcutText.font
                                        text:   conversationsShortcutText.text
                                    }
                                }
                            }
                        }
                    }
                }
                Item {
                    id: tabTwo
                    Rectangle {
                        implicitWidth: parent.width / 2
                        implicitHeight: parent.height
                        anchors.left: parent.left
                        TableView {
                            id: callsTableView
                            anchors.fill: parent
                            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                            frameVisible: false
                            TableViewColumn {
                                role: "Description"
                                width: callsTableView.width / 2
                                delegate: descriptionDelegate
                            }
                            TableViewColumn {
                                role: "Shortcut"
                                width: callsTableView.width / 2
                                delegate: Component{
                                    Loader {
                                        property variant modelData: model
                                        sourceComponent: shortcutDelegateWithThreeKeys
                                        asynchronous: true
                                    }
                                }
                            }
                            model: keyboardCallsShortcutsModel
                            rowDelegate: Rectangle {
                                height: 50
                                color: JamiTheme.backgroundColor
                            }
                            style: TableViewStyle {
                                backgroundColor: JamiTheme.backgroundColor
                                alternateBackgroundColor: JamiTheme.backgroundColor
                                headerDelegate: Rectangle {
                                    // Only first column's header is shown
                                    height: [t_metrics_calls.tightBoundingRect.height + 10, 0][styleData.column % 2]
                                    width: [parent.width, 0][styleData.column % 2]
                                    color: JamiTheme.backgroundColor
                                    radius: 4
                                    anchors.top: parent.top
                                    anchors.topMargin: 5
                                    Text {
                                        id : callsShortcutText
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        font.family: "Arial"
                                        font.pointSize: 12
                                        text: styleData.column % 2 ? "" : "Calls"
                                        color: JamiTheme.textColor
                                    }
                                    // make sure that calls and settings header are parallel
                                    TextMetrics {
                                        id:     t_metrics_calls
                                        font:   callsShortcutText.font
                                        text:   "Settings"
                                    }
                                }
                            }
                        }
                    }
                    Rectangle {
                        implicitWidth: parent.width / 2
                        implicitHeight: parent.height
                        anchors.right: parent.right
                        TableView {
                            id: settingsTableView
                            anchors.fill: parent
                            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                            frameVisible: false
                            TableViewColumn {
                                role: "Description"
                                width: settingsTableView.width / 2
                                delegate: descriptionDelegate
                            }
                            TableViewColumn {
                                role: "Shortcut"
                                width: settingsTableView.width / 2
                                delegate: Component{
                                    Loader {
                                        property variant modelData: model
                                        sourceComponent: shortcutDelegateWithThreeKeys
                                        asynchronous: true
                                    }
                                }
                            }
                            model: keyboardSettingsShortcutsModel
                            rowDelegate: Rectangle {
                                height: 50
                                color: JamiTheme.backgroundColor
                            }
                            style: TableViewStyle {
                                backgroundColor: JamiTheme.backgroundColor
                                alternateBackgroundColor: JamiTheme.backgroundColor
                                headerDelegate: Rectangle {
                                    // Only first column's header is shown
                                    height: [t_metrics_settings.tightBoundingRect.height + 10, 0][styleData.column % 2]
                                    width: [parent.width, 0][styleData.column % 2]
                                    color: JamiTheme.backgroundColor
                                    radius: 4
                                    anchors.top: parent.top
                                    anchors.topMargin: 5
                                    Text {
                                        id : settingsShortcutText
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        font.family: "Arial"
                                        font.pointSize: 12
                                        text: styleData.column % 2 ? "" : "Settings"
                                        color: JamiTheme.textColor
                                    }
                                    TextMetrics {
                                        id:     t_metrics_settings
                                        font:   settingsShortcutText.font
                                        text:   settingsShortcutText.text
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        TabBar {
            id: tabBar
            anchors.horizontalCenter: parent.horizontalCenter

            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height - 500  // Forced postion below table

            width: JamiTheme.preferredFieldWidth * 2
            height: JamiTheme.preferredFieldHeight
            background: Rectangle { color: "transparent" }

            currentIndex: 0
            TabButton {
                id: pageOne
                width: JamiTheme.preferredFieldWidth
                text: "1"
                down: true
                // customize tab button
                contentItem: Text {
                    text: pageOne.text
                    font: pageOne.font
                    opacity: enabled ? 1.0 : 0.3
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    color: JamiTheme.textColor
                }
                // customize tab button
                background: Rectangle {
                    id: buttonRectOne
                    implicitWidth: JamiTheme.preferredFieldWidth
                    implicitHeight: JamiTheme.preferredFieldHeight
                    radius: 4
                    color: pageOne.down ? JamiTheme.selectedColor : "transparent"
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onPressed: { buttonRectOne.color = JamiTheme.pressColor; tabBar.currentIndex = 0; pageOne.down = true; pageTwo.down = false;}
                        onReleased: { buttonRectOne.color = JamiTheme.selectedColor; }
                        onEntered: { buttonRectOne.color = JamiTheme.hoverColor; }
                        onExited: { buttonRectOne.color = Qt.binding(function() { return pageOne.down ? JamiTheme.selectedColor : "transparent" }); }
                    }
                }
            }
            TabButton {
                id: pageTwo
                text: "2"
                width: JamiTheme.preferredFieldWidth
                contentItem: Text {
                    text: pageTwo.text
                    font: pageTwo.font
                    opacity: enabled ? 1.0 : 0.3
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    color: JamiTheme.textColor
                }

                background: Rectangle {
                    id: buttonRectTwo
                    implicitWidth: JamiTheme.preferredFieldWidth
                    implicitHeight: JamiTheme.preferredFieldHeight

                    radius: 4
                    color: pageTwo.down ? JamiTheme.selectedColor : "transparent"
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onPressed: { buttonRectTwo.color = JamiTheme.pressColor; tabBar.currentIndex = 1; pageTwo.down = true; pageOne.down = false;}
                        onReleased: { buttonRectTwo.color = JamiTheme.selectedColor; }
                        onEntered: { buttonRectTwo.color = JamiTheme.hoverColor; }
                        onExited: { buttonRectTwo.color = Qt.binding(function() { return pageTwo.down ? JamiTheme.selectedColor : "transparent" }); }
                    }
                }
            }
        }

        MaterialButton {
            id: btnClose

            anchors.bottom: parent.bottom
            anchors.bottomMargin: JamiTheme.preferredMarginSize
            anchors.horizontalCenter: parent.horizontalCenter

            width: JamiTheme.preferredFieldWidth / 2
            height: JamiTheme.preferredFieldHeight

            color: JamiTheme.buttonTintedBlack
            hoveredColor: JamiTheme.buttonTintedBlackHovered
            pressedColor: JamiTheme.buttonTintedBlackPressed
            outlined: true

            text: JamiStrings.close

            onClicked: {
                close()
            }
        }
    }
}
