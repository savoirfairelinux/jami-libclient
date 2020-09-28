/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.12
import net.jami.Models 1.0

import "../../commoncomponents"

Popup {
    id: contactPickerPopup

    property int type: ContactPicker.ContactPickerType.JAMICONFERENCE


    // Important to keep it one, since enum in c++ starts at one for conferences.
    enum ContactPickerType {
        JAMICONFERENCE = 1,
        SIPTRANSFER
    }

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

            source: "qrc:/images/icons/ic_close_black_24dp.png"

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

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: type === ContactPicker.ContactPickerType.JAMICONFERENCE ? qsTr("Add to conference") : qsTr("Transfer this call")
            }

            ContactSearchBar {
                id: contactPickerContactSearchBar

                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 5
                Layout.bottomMargin: 5
                Layout.preferredWidth: contactPickerPopupRect.width - 10
                Layout.preferredHeight: 35

                onContactSearchBarTextChanged: {
                    ContactAdapter.setSearchFilter(text)
                }

                Component.onCompleted: {
                    contactPickerContactSearchBar.setPlaceholderString(
                                qsTr("Search contacts"))
                }
            }

            ListView {
                id: contactPickerListView

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: contactPickerPopupRect.width
                Layout.preferredHeight: 200

                model: ContactAdapter.getContactSelectableModel(type)

                clip: true

                delegate: ContactPickerItemDelegate {
                    id: contactPickerItemDelegate
                }

                ScrollIndicator.vertical: ScrollIndicator {}
            }
        }

        radius: 10
        color: "white"
    }

    onAboutToShow: {
         // Reset the model on each show.
        contactPickerListView.model = ContactAdapter.getContactSelectableModel(
                    type)
    }

    background: Rectangle {
        color: "transparent"
    }
}
