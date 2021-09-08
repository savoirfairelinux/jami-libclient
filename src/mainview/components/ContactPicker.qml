/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Popup {
    id: contactPickerPopup

    property int type: ContactList.CONFERENCE

    contentWidth: 250
    contentHeight: contactPickerPopupRectColumnLayout.height + 50

    padding: 0

    modal: true

    contentItem: Rectangle {
        id: contactPickerPopupRect
        width: 250

        PushButton {
            id: closeButton

            anchors.top: contactPickerPopupRect.top
            anchors.topMargin: 5
            anchors.right: contactPickerPopupRect.right
            anchors.rightMargin: 5
            imageColor: JamiTheme.textColor

            source: JamiResources.round_close_24dp_svg

            onClicked: {
                contactPickerPopup.close()
            }
        }

        ColumnLayout {
            id: contactPickerPopupRectColumnLayout

            anchors.top: contactPickerPopupRect.top
            anchors.topMargin: 15

            Text {
                id: contactPickerTitle

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: contactPickerPopupRect.width
                Layout.preferredHeight: 30

                font.pointSize: JamiTheme.textFontSize
                font.bold: true
                color: JamiTheme.textColor

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: {
                    switch(type) {
                    case ContactList.CONFERENCE:
                        return qsTr("Add to conference")
                    case ContactList.TRANSFER:
                        return qsTr("Transfer this call")
                    default:
                        return qsTr("Add default moderator")
                    }
                }
            }

            ContactSearchBar {
                id: contactPickerContactSearchBar

                Layout.alignment: Qt.AlignCenter
                Layout.margins: 5
                Layout.fillWidth: true
                Layout.preferredHeight: 35

                placeHolderText: JamiStrings.addParticipant

                onContactSearchBarTextChanged: {
                    ContactAdapter.setSearchFilter(text)
                }
            }

            JamiListView {
                id: contactPickerListView

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: contactPickerPopupRect.width
                Layout.preferredHeight: 200

                model: ContactAdapter.getContactSelectableModel(type)

                delegate: ContactPickerItemDelegate {
                    id: contactPickerItemDelegate

                    showPresenceIndicator: type !== ContactList.TRANSFER
                }
            }
        }

        radius: 10
        color: JamiTheme.backgroundColor
    }

    onAboutToShow: {
        contactPickerListView.model =
                ContactAdapter.getContactSelectableModel(type)
    }

    background: Rectangle {
        color: "transparent"
    }
}
