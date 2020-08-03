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

Dialog {
    id: deleteAccountDialog

    property int profileType: ClientWrapper.settingsAdaptor.getCurrentAccount_Profile_Info_Type()

    property bool isSIP: {
        switch (profileType) {
        case Profile.Type.SIP:
            return true;
        default:
            return false;
        }
    }

    onOpened: {
        profileType = ClientWrapper.settingsAdaptor.getCurrentAccount_Profile_Info_Type()
        labelBestId.text = ClientWrapper.settingsAdaptor.getAccountBestName()
        labelAccountHash.text = ClientWrapper.settingsAdaptor.getCurrentAccount_Profile_Info_Uri()
    }

    onVisibleChanged: {
        if(!visible){
            reject()
        }
    }

    visible: false
    title: qsTr("Account deletion")

    contentItem: Rectangle{
        implicitWidth: 400
        implicitHeight: 300

        ColumnLayout{
            anchors.fill: parent
            spacing: 7

            Layout.alignment: Qt.AlignCenter

            Label{
                id: labelDeletion

                Layout.topMargin: 11
                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 8
                font.kerning: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                text:qsTr("Do you really want to delete the following account?")
            }

            Label{
                id: labelBestId

                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 8
                font.kerning: true
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap

                text: ClientWrapper.settingsAdaptor.getAccountBestName()
            }

            Label{
                id: labelAccountHash

                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 8
                font.kerning: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                text: ClientWrapper.settingsAdaptor.getCurrentAccount_Profile_Info_Uri()
            }

            Item{
                Layout.fillWidth: true

                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Layout.maximumHeight: 5
                Layout.preferredHeight: 5
                Layout.minimumHeight: 5
            }

            Label{
                id: labelWarning

                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Layout.preferredWidth: 300

                visible: ! isSIP

                Layout.alignment: Qt.AlignHCenter
                wrapMode: Text.Wrap
                text: qsTr("If this account hasn't been exported, or added to another device, it will be irrevocably lost.")
                font.pointSize: 8
                font.kerning: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: "red"
            }

            Item{
                Layout.fillWidth: true

                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Layout.maximumHeight: 10
                Layout.preferredHeight: 10
                Layout.minimumHeight: 10
            }

            RowLayout{
                spacing: 7

                Layout.leftMargin: 11
                Layout.rightMargin: 11

                Item{
                    Layout.fillWidth: true
                    Layout.minimumWidth: 40

                    Layout.maximumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.minimumHeight: 20
                }

                HoverableRadiusButton{
                    id: btnDeleteAccept

                    Layout.maximumWidth: 130
                    Layout.preferredWidth: 130
                    Layout.minimumWidth: 130

                    Layout.maximumHeight: 30
                    Layout.preferredHeight: 30
                    Layout.minimumHeight: 30

                    radius: height /2

                    text: qsTr("Delete")
                    font.pointSize: 10
                    font.kerning: true

                    onClicked: {
                        ClientWrapper.accountAdaptor.deleteCurrentAccount()
                        accept()
                    }
                }

                Item{
                    Layout.fillWidth: true
                    Layout.minimumWidth: 40

                    Layout.maximumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.minimumHeight: 20
                }

                HoverableButtonTextItem{
                    id: btnDeleteCancel

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
                Layout.fillWidth: true

                Layout.maximumHeight: 5
                Layout.preferredHeight: 5
                Layout.minimumHeight: 5

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.bottomMargin: 11
            }
        }
    }
}
