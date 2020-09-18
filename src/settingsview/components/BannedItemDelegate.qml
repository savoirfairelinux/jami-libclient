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
    id: root

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

    RowLayout {
        anchors.fill: parent

        Label {
            id: labelContactAvatar

            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            background: Rectangle {
                anchors.fill: parent
                color: "transparent"
                Image {
                    id: avatarImg

                    anchors.fill: parent
                    source: contactPicture_base64 === "" ? "" : "data:image/png;base64," + contactPicture_base64
                    fillMode: Image.PreserveAspectCrop
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
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

        ColumnLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter

            Label{
                id: labelContactName

                Layout.fillWidth: true

                Layout.preferredHeight: 24

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: contactName === "" ? JamiStrings.name : contactName
            }

            Label{
                id: labelContactId

                Layout.fillWidth: true

                Layout.minimumHeight: 24
                Layout.preferredHeight: 24
                Layout.maximumHeight: 24

                font.pointSize: JamiTheme.textFontSize
                font.kerning: true

                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                elide: Text.ElideRight
                text: contactID === "" ? JamiStrings.identifier : contactID
            }
        }

        HoverableRadiusButton{
            id: btnReAddContact

            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.rightMargin: 8

            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            buttonImageHeight: height - 8
            buttonImageWidth: width - 8

            source: "qrc:/images/icons/person_add-24px.svg"

            radius: height / 2
            width: 25
            height: 25

            backgroundColor: "transparent"

            ToolTip.visible: hovered
            ToolTip.text: JamiStrings.reinstateContact

            onClicked: {
                btnReAddContactClicked()
            }
        }
    }
}

