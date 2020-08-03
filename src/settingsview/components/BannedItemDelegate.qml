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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0

import "../../commoncomponents"

ItemDelegate {
    id: deviceItemDelegate

    property string contactName : ""
    property string contactID: ""
    property string contactPicture_base64:""

    signal btnReAddContactClicked

    function btnReAddContactEnter(){
        btnReAddContact.enterBtn()
    }

    function btnReAddContactExit(){
        btnReAddContact.exitBtn()
    }

    function btnReAddContactPress(){
        btnReAddContact.pressBtn()
    }

    function btnReAddContactRelease(){
        btnReAddContact.releaseBtn()
    }

    highlighted: ListView.isCurrentItem

    RowLayout{
        anchors.fill: parent

        spacing: 7

        Label{
            id: labelContactAvatar

            Layout.alignment: Qt.AlignVCenter

            Layout.leftMargin: 7
            Layout.topMargin: 7
            Layout.bottomMargin: 7

            Layout.minimumWidth: 48
            Layout.preferredWidth: 48
            Layout.maximumWidth: 48

            Layout.minimumHeight: 48
            Layout.preferredHeight: 48
            Layout.maximumHeight: 48

            background: Rectangle{
                anchors.fill: parent
                color: "transparent"
                Image {
                    id: avatarImg

                    anchors.fill: parent
                    source: "data:image/png;base64," + contactPicture_base64
                    fillMode: Image.PreserveAspectCrop
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle{
                            width: avatarImg.width
                            height: avatarImg.height
                            radius: {
                                var size = ((avatarImg.width <= avatarImg.height)? avatarImg.width:avatarImg.height)
                                return size /2
                            }
                        }
                    }
                }
            }
        }

        Item{
            Layout.minimumWidth: 8
            Layout.preferredWidth: 8
            Layout.maximumWidth: 8

            Layout.minimumHeight: 20
            Layout.preferredHeight: 20
            Layout.maximumHeight: 20
        }

        ColumnLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.topMargin: 7
            Layout.bottomMargin: 7

            Layout.alignment: Qt.AlignVCenter

            spacing: 7

            Label{
                id: labelContactName

                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.maximumWidth: 16777215

                Layout.minimumHeight: 30
                Layout.preferredHeight: 30
                Layout.maximumHeight: 30

                font.pointSize: 8
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                text: contactName === "" ? qsTr("name") : contactName
            }

            Label{
                id: labelContactId

                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.maximumWidth: 16777215

                Layout.minimumHeight: 30
                Layout.preferredHeight: 30
                Layout.maximumHeight: 30

                font.pointSize: 8
                font.kerning: true

                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter

                text: contactID === "" ? qsTr("id") : contactID
            }
        }

        HoverableRadiusButton{
            id: btnReAddContact

            Layout.topMargin: 7
            Layout.bottomMargin: 7

            Layout.minimumWidth: 30
            Layout.preferredWidth: 30
            Layout.maximumWidth: 30

            Layout.minimumHeight: 30
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30

            font.pointSize: 8
            font.kerning: true

            buttonImageHeight: height
            buttonImageWidth: height

            source:"qrc:/images/icons/ic_person_add_black_24dp_2x.png"

            ToolTip.visible: isHovering
            ToolTip.text: qsTr("Add as contact")

            onClicked: {
                btnReAddContactClicked()
            }
        }

        Item{
            Layout.rightMargin: 7

            Layout.minimumWidth: 8
            Layout.preferredWidth: 8
            Layout.maximumWidth: 8

            Layout.minimumHeight: 20
        }
    }
}

