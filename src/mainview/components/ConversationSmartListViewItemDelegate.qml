
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
import net.jami.Models 1.0

import "../../commoncomponents"

ItemDelegate {
    id: smartListItemDelegate
    height: 72

    property int lastInteractionPreferredWidth: 80
    Connections {
        target: conversationSmartListView


        /*
         * Hack, make sure that smartListItemDelegate does not show extra item
         * when searching new contacts.
         */
        function onForceUpdatePotentialInvalidItem() {
            smartListItemDelegate.visible = conversationSmartListView.model.rowCount(
                        ) <= index ? false : true
        }


        /*
         * When currentIndex is -1, deselect items, if not, change select item
         */
        function onCurrentIndexIsChanged() {
            if (conversationSmartListView.currentIndex === -1
                    || conversationSmartListView.currentIndex !== index) {
                itemSmartListBackground.color = Qt.binding(function () {
                    return InCall ? Qt.lighter(JamiTheme.selectionBlue,
                                               1.8) : JamiTheme.backgroundColor
                })
            } else {
                itemSmartListBackground.color = Qt.binding(function () {
                    return InCall ? Qt.lighter(JamiTheme.selectionBlue,
                                               1.8) : JamiTheme.releaseColor
                })
            }
        }

        function onNeedToShowChatView(accountId, convUid) {
            if (convUid === UID) {
                conversationSmartListView.needToAccessMessageWebView(
                            DisplayID == DisplayName ? "" : DisplayID,
                            DisplayName, UID, CallStackViewShouldShow,
                            IsAudioOnly, CallStateStr)
            }
        }
    }

    ConversationSmartListUserImage {
        id: conversationSmartListUserImage

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 16
    }


    RowLayout {
        id: rowUsernameAndLastInteractionDate
        anchors.left: conversationSmartListUserImage.right
        anchors.leftMargin: 16
        anchors.top: parent.top
        anchors.topMargin: conversationSmartListUserLastInteractionMessage.text !== "" ?
                               16 : parent.height/2-conversationSmartListUserName.height/2
        anchors.right: parent.right
        anchors.rightMargin: 10

        Text {
            id: conversationSmartListUserName
            Layout.alignment: conversationSmartListUserLastInteractionMessage.text !== "" ?
                                  Qt.AlignLeft : Qt.AlignLeft | Qt.AlignVCenter

            TextMetrics {
                id: textMetricsConversationSmartListUserName
                font: conversationSmartListUserName.font
                elide: Text.ElideRight
                elideWidth: LastInteractionDate ? (smartListItemDelegate.width - lastInteractionPreferredWidth - conversationSmartListUserImage.width-32) :
                                                  smartListItemDelegate.width - lastInteractionPreferredWidth
                text: DisplayName
            }
            text: textMetricsConversationSmartListUserName.elidedText
            font.pointSize: JamiTheme.menuFontSize
        }

        Text {
            id: conversationSmartListUserLastInteractionDate
            Layout.alignment: Qt.AlignRight
            TextMetrics {
                id: textMetricsConversationSmartListUserLastInteractionDate
                font: conversationSmartListUserLastInteractionDate.font
                elide: Text.ElideRight
                elideWidth: lastInteractionPreferredWidth
                text: LastInteractionDate
            }

            text: textMetricsConversationSmartListUserLastInteractionDate.elidedText
            font.pointSize: JamiTheme.textFontSize
            color: JamiTheme.faddedLastInteractionFontColor
        }
    }


    Text {
        id: conversationSmartListUserLastInteractionMessage
        anchors.left: conversationSmartListUserImage.right
        anchors.leftMargin: 16
        anchors.bottom: rowUsernameAndLastInteractionDate.bottom
        anchors.bottomMargin: -20

        TextMetrics {
            id: textMetricsConversationSmartListUserLastInteractionMessage
            font: conversationSmartListUserLastInteractionMessage.font
            elide: Text.ElideRight
            elideWidth: LastInteractionDate ? (smartListItemDelegate.width - lastInteractionPreferredWidth - conversationSmartListUserImage.width-32) :
                                              smartListItemDelegate.width - lastInteractionPreferredWidth
            text: InCall ? CallStateStr : (Draft ? Draft : LastInteraction)
        }

        font.hintingPreference: Font.PreferNoHinting
        text: textMetricsConversationSmartListUserLastInteractionMessage.elidedText
        font.pointSize: JamiTheme.textFontSize
        color: Draft ? JamiTheme.draftRed : JamiTheme.faddedLastInteractionFontColor
    }



    background: Rectangle {
        id: itemSmartListBackground
        color: InCall ? Qt.lighter(JamiTheme.selectionBlue, 1.8) : JamiTheme.backgroundColor
        implicitWidth: conversationSmartListView.width
        implicitHeight: parent.height
        border.width: 0
    }

    MouseArea {
        id: mouseAreaSmartListItemDelegate

        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: {
            if (!InCall) {
                itemSmartListBackground.color = JamiTheme.pressColor
            }
        }
        onDoubleClicked: {
            if (!InCall) {
                ConversationsAdapter.selectConversation(ClientWrapper.utilsAdaptor.getCurrAccId(),
                                                        UID, false)
                CallAdapter.placeCall()
            }
        }
        onReleased: {
            if (!InCall) {
                itemSmartListBackground.color = JamiTheme.releaseColor
            }
            if (mouse.button === Qt.RightButton) {


                /*
                 * Make menu pos at mouse.
                 */
                var relativeMousePos = mapToItem(itemSmartListBackground,
                                                 mouse.x, mouse.y)
                smartListContextMenu.x = relativeMousePos.x
                smartListContextMenu.y = relativeMousePos.y
                smartListContextMenu.responsibleAccountId = ClientWrapper.utilsAdaptor.getCurrAccId()
                smartListContextMenu.responsibleConvUid = UID
                userProfile.responsibleConvUid = UID
                userProfile.aliasText = DisplayName
                userProfile.registeredNameText = DisplayID
                userProfile.idText = URI
                userProfile.contactPicBase64 = Picture
                smartListContextMenu.openMenu()
            } else if (mouse.button === Qt.LeftButton) {
                conversationSmartListView.currentIndex = index
                conversationSmartListView.needToSelectItems(UID)
                conversationSmartListView.needToGrabFocus()
            }
        }
        onEntered: {
            if (!InCall) {
                itemSmartListBackground.color = JamiTheme.hoverColor
            }
        }
        onExited: {
            if (!InCall) {
                if (conversationSmartListView.currentIndex !== index
                        || conversationSmartListView.currentIndex === -1) {
                    itemSmartListBackground.color = Qt.binding(function () {
                        return InCall ? Qt.lighter(JamiTheme.selectionBlue,
                                                   1.8) : JamiTheme.backgroundColor
                    })
                } else {
                    itemSmartListBackground.color = Qt.binding(function () {
                        return InCall ? Qt.lighter(JamiTheme.selectionBlue,
                                                   1.8) : JamiTheme.releaseColor
                    })
                }
            }
        }
    }

    ConversationSmartListContextMenu {
        id: smartListContextMenu
    }
}
