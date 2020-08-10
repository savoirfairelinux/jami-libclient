/**
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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
    id: mediahandlerPickerPopup

    function toggleMediaHandlerSlot(mediaHandlerId, isLoaded) {
        ClientWrapper.pluginModel.toggleCallMediaHandler(mediaHandlerId, !isLoaded)
        mediahandlerPickerListView.model = MediaHandlerAdapter.getMediaHandlerSelectableModel()
    }

    contentWidth: 350
    contentHeight: mediahandlerPickerPopupRectColumnLayout.height + 50

    padding: 0

    modal: true

    contentItem: Rectangle {
        id: mediahandlerPickerPopupRect

        width: 250

        HoverableButton {
            id: closeButton

            anchors.top: mediahandlerPickerPopupRect.top
            anchors.topMargin: 5
            anchors.right: mediahandlerPickerPopupRect.right
            anchors.rightMargin: 5

            width: 30
            height: 30

            radius: 30
            source: "qrc:/images/icons/round-close-24px.svg"

            onClicked: {
                mediahandlerPickerPopup.close()
            }
        }

        ColumnLayout {
            id: mediahandlerPickerPopupRectColumnLayout

            anchors.top: mediahandlerPickerPopupRect.top
            anchors.topMargin: 15

            Text {
                id: mediahandlerPickerTitle

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: mediahandlerPickerPopupRect.width
                Layout.preferredHeight: 30

                font.pointSize: JamiTheme.textFontSize
                font.bold: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: qsTr("Choose plugin")
            }

            ListView {
                id: mediahandlerPickerListView

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: mediahandlerPickerPopupRect.width
                Layout.preferredHeight: 200

                model: MediaHandlerAdapter.getMediaHandlerSelectableModel()

                clip: true

                delegate: MediaHandlerItemDelegate {
                    id: mediaHandlerItemDelegate
                    visible: ClientWrapper.pluginModel.getPluginsEnabled()
                    width: mediahandlerPickerListView.width
                    height: 50

                    mediaHandlerName : MediaHandlerName
                    mediaHandlerId: MediaHandlerId
                    mediaHandlerIcon: MediaHandlerIcon
                    isLoaded: IsLoaded

                    onBtnLoadMediaHandlerToggled: {
                        toggleMediaHandlerSlot(mediaHandlerId, isLoaded)
                    }

                }

                ScrollIndicator.vertical: ScrollIndicator {}
            }
        }

        radius: 10
        color: "white"
    }

    onAboutToShow: {
        // Reset the model on each show.
        mediahandlerPickerListView.model = MediaHandlerAdapter.getMediaHandlerSelectableModel()
    }

    background: Rectangle {
        color: "transparent"
    }
}
