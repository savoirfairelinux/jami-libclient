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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14

import net.jami.Adapters 1.0
import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Popup {
    id: contactPickerPopup

    property int type: ContactPicker.ContactPickerType.JAMICONFERENCE


    // Important to keep it one, since enum in c++ starts at one for conferences.
    enum ContactPickerType {
        CONVERSATION = 0,
        JAMICONFERENCE,
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
            imageColor: JamiTheme.textColor

            source: "qrc:/images/icons/round-close-24px.svg"

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
                    case ContactPicker.ContactPickerType.JAMICONFERENCE:
                        return qsTr("Add to conference")
                    case ContactPicker.ContactPickerType.SIPTRANSFER:
                        return qsTr("Transfer this call")
                    default:
                        return qsTr("Add default moderator")
                    }
                }
            }

            ContactSearchBar {
                id: contactPickerContactSearchBar

                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 5
                Layout.bottomMargin: 5
                Layout.preferredWidth: implicitWidth
                Layout.preferredHeight: 35

                implicitWidth: contactPickerPopupRect.width - 10

                onContactSearchBarTextChanged: {
                    ContactAdapter.setSearchFilter(text)
                }
            }

            ListViewJami {
                id: contactPickerListView

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: contactPickerPopupRect.width
                Layout.preferredHeight: 200
                border.width: 0

                model: ContactAdapter.getContactSelectableModel(type)

                clip: true

                delegate: ContactPickerItemDelegate {
                    id: contactPickerItemDelegate
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
