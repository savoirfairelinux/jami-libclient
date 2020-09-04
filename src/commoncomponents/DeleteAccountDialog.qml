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
import net.jami.Adapters 1.0
import "../constant"

Dialog {
    id: root

    property int profileType: SettingsAdapter.getCurrentAccount_Profile_Info_Type()

    property bool isSIP: {
        switch (profileType) {
        case Profile.Type.SIP:
            return true;
        default:
            return false;
        }
    }

    onOpened: {
        profileType = SettingsAdapter.getCurrentAccount_Profile_Info_Type()
        labelBestId.text = SettingsAdapter.getAccountBestName()
        labelAccountHash.text = SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
    }

    onVisibleChanged: {
        if(!visible){
            reject()
        }
    }

    header : Rectangle {
        width: parent.width
        height: 64
        color: "transparent"
        Text {
            anchors.fill: parent
            anchors.leftMargin: JamiTheme.preferredMarginSize
            anchors.topMargin: JamiTheme.preferredMarginSize

            text: qsTr("Account deletion")
            font.pointSize: JamiTheme.headerFontSize
            wrapMode: Text.Wrap
        }
    }

    visible: false
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    contentItem: Rectangle {
        implicitHeight: contentLayout.implicitHeight + 64 + JamiTheme.preferredMarginSize
        implicitWidth: 350

        ColumnLayout{
            id: contentLayout
            anchors.fill: parent
            anchors.centerIn: parent

            Label {
                id: labelDeletion

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                Layout.alignment: Qt.AlignHCenter

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text:qsTr("Do you really want to delete the following account?")
            }

            Label {
                id: labelBestId

                Layout.topMargin: 5
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                Layout.alignment: Qt.AlignHCenter

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap

                text: SettingsAdapter.getAccountBestName()
            }

            Label {
                id: labelAccountHash

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 30

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text: SettingsAdapter.getCurrentAccount_Profile_Info_Uri()
            }

            Label {
                id: labelWarning

                Layout.topMargin: 5
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                visible: ! isSIP

                wrapMode: Text.Wrap
                text: qsTr("If this account hasn't been exported, or added to another device, it will be irrevocably lost.")
                font.pointSize: JamiTheme.textFontSize
                font.kerning: true
                horizontalAlignment: Text.AlignHCenter
                color: "red"
            }

            RowLayout {
                Layout.topMargin: JamiTheme.preferredMarginSize / 2
                Layout.bottomMargin: 5
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                HoverableRadiusButton {
                    id: btnDeleteAccept

                    Layout.fillWidth: true
                    Layout.maximumWidth: 130

                    radius: height / 2

                    text: qsTr("Delete")
                    font.pointSize: JamiTheme.buttonFontSize
                    font.kerning: true

                    onClicked: {
                        ClientWrapper.accountAdaptor.deleteCurrentAccount()
                        accept()
                    }
                }

                HoverableButtonTextItem {
                    id: btnDeleteCancel

                    Layout.leftMargin: 20
                    Layout.fillWidth: true
                    Layout.maximumWidth: 130

                    backgroundColor: "red"
                    onEnterColor: Qt.rgba(150 / 256, 0, 0, 0.7)
                    onDisabledBackgroundColor: Qt.rgba(
                                                   255 / 256,
                                                   0, 0, 0.8)
                    onPressColor: backgroundColor
                    textColor: "white"

                    radius: height / 2

                    text: qsTr("Cancel")
                    font.pointSize: JamiTheme.buttonFontSize
                    font.kerning: true

                    onClicked: {
                        reject()
                    }
                }
            }
        }
    }
}
