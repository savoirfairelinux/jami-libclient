/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
 * Author: Sébastien blin <sebastien.blin@savoirfairelinux.com>
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.15
import net.jami.Models 1.0

import "../../constant"
import "../../commoncomponents"

ColumnLayout {
    anchors.fill: parent
    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter

    signal welcomePageRedirectPage(int toPageIndex)
    signal leavePage

    Item {
        // put a spacer to make the buttons closs to the middle
        Layout.preferredHeight: 48
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Label {
            id: welcomeLabel
            Layout.maximumHeight: 40
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Welcome to")
            font.pointSize: 30
            font.kerning: true
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Label {
            id: welcomeLogo
            Layout.alignment: Qt.AlignCenter
            Layout.minimumWidth: 100
            Layout.minimumHeight: 100
            Layout.maximumWidth: 16777215
            Layout.maximumHeight: 16777215
            Layout.preferredWidth: 300
            Layout.preferredHeight: 150
            color: "transparent"
            background: Image {
                id: logoIMG
                source: "qrc:/images/logo-jami-standard-coul.png"
                fillMode: Image.PreserveAspectFit
                mipmap: true
            }
        }
    }
    RowLayout {
        spacing: 8
        Layout.fillWidth: true
        Layout.maximumHeight: 36
        Layout.alignment: Qt.AlignHCenter
        MaterialButton {
            id: newAccountButton

            text: qsTr("CREATE A JAMI ACCOUNT")
            toolTipText: qsTr("Create new Jami account")
            source: "qrc:/images/default_avatar_overlay.svg"
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                welcomePageRedirectPage(1)
            }
        }
    }
    RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Layout.maximumHeight: 36
        Layout.alignment: Qt.AlignHCenter
        MaterialButton {
            id: fromDeviceButton

            text: qsTr("IMPORT FROM ANOTHER DEVICE")
            toolTipText: qsTr("Import account from other device")
            source: "qrc:/images/icons/devices-24px.svg"
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                welcomePageRedirectPage(5)
            }
        }
    }
    RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Layout.maximumHeight: 36
        Layout.alignment: Qt.AlignHCenter
        MaterialButton {
            id: fromBackupButton

            text: qsTr("CONNECT FROM BACKUP")
            toolTipText: qsTr("Import account from backup file")
            source: "qrc:/images/icons/backup-24px.svg"
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                welcomePageRedirectPage(3)
            }
        }
    }
    RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Layout.maximumHeight: 36
        Layout.alignment: Qt.AlignHCenter
        MaterialButton {
            id: showAdvancedButton

            text: qsTr("SHOW ADVANCED")
            toolTipText: qsTr("Show advanced options")
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed
            outlined: true

            hoverEnabled: true

            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Show advanced options")

            onClicked: {
                connectAccountManagerButton.visible = !connectAccountManagerButton.visible
                newSIPAccountButton.visible = !newSIPAccountButton.visible
            }
        }
    }
    RowLayout {
        spacing: 8
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        Layout.maximumHeight: 36
        MaterialButton {
            id: connectAccountManagerButton
            visible: false

            text: qsTr("CONNECT TO MANAGEMENT SERVER")
            toolTipText: qsTr("Login to account manager")
            source: "qrc:/images/icons/router-24px.svg"
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                welcomePageRedirectPage(6)
            }
        }
    }
    RowLayout {
        spacing: 8
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumHeight: 36
        MaterialButton {
            id: newSIPAccountButton
            visible: false

            text: qsTr("CREATE A SIP ACCOUNT")
            toolTipText: qsTr("Create new SIP account")
            source: "qrc:/images/default_avatar_overlay.svg"
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed

            onClicked: {
                welcomePageRedirectPage(2)
            }
        }
    }
    Item {
        // put a spacer to make the buttons closs to the middle
        Layout.fillHeight: true
        Layout.preferredHeight: 65
        Layout.fillWidth: true
    }

    HoverableButton {
        id: cancelButton
        z: 2

        visible: {
            return ClientWrapper.utilsAdaptor.getAccountListSize() > 0
        }

        anchors.right: parent.right
        anchors.top: parent.top

        rightPadding: 40
        topPadding: 40

        Layout.preferredWidth: 96
        Layout.preferredHeight: 96

        backgroundColor: "transparent"
        onEnterColor: "transparent"
        onPressColor: "transparent"
        onReleaseColor: "transparent"
        onExitColor: "transparent"

        buttonImageHeight: 48
        buttonImageWidth: 48
        source: "qrc:/images/icons/ic_close_white_24dp.png"
        radius: 48
        baseColor: "#7c7c7c"
        toolTipText: qsTr("Close")

        Action {
            enabled: parent.visible
            shortcut: StandardKey.Cancel
            onTriggered: leavePage()
        }

        onClicked: {
            leavePage()
        }
    }
}
