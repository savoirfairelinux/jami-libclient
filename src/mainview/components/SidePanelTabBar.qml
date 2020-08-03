
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

TabBar {
    id: tabBar

    property alias converstationTabDown: pageOne.down
    property alias invitationTabDown: pageTwo.down
    property alias converstationTabWidth: pageOne.width
    property alias invitationTabWidth: pageTwo.width
    property alias converstationTabHeight: pageOne.height
    property alias invitationTabHeight: pageTwo.height

    visible: tabBarVisible

    currentIndex: 0

    TabButton {
        id: pageOne

        width: tabBar.width / 2 - tabButtonShrinkSize
        height: textConvElement.height + 10

        down: true

        Rectangle {
            id: totalUnreadMessagesCountRect

            anchors.right: pageOne.right
            anchors.rightMargin: 5
            anchors.bottom: pageOne.bottom
            anchors.bottomMargin: pageOne.height - totalUnreadMessagesCountRect.height / 2

            width: 14
            height: 14

            visible: totalUnreadMessagesCount > 0

            Text {
                id: totalUnreadMessagesCountText

                anchors.centerIn: totalUnreadMessagesCountRect

                text: totalUnreadMessagesCount > 9 ? "···" : totalUnreadMessagesCount
                color: "white"
            }

            radius: 30
            color: JamiTheme.notificationRed
        }

        background: Rectangle {
            id: buttonRectOne

            radius: 10
            width: pageOne.width + 2
            color: pageOne.down ? "white" : JamiTheme.releaseColor
            border.color: JamiTheme.tabbarBorderColor

            Text {
                id: textConvElement

                anchors.centerIn: buttonRectOne

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: qsTr("Converstation")
                font.pointSize: JamiTheme.textFontSize
                opacity: enabled ? 1.0 : 0.3
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPressed: {
                    buttonRectOne.color = JamiTheme.pressColor
                    ConversationsAdapter.setConversationFilter("")
                    contactSearchBar.setPlaceholderString(
                                JamiTheme.contactSearchBarPlaceHolderConversationText)
                    pageOne.down = true
                    pageTwo.down = false
                    setCurrentUidSmartListModelIndex()
                    forceReselectConversationSmartListCurrentIndex()
                }
                onReleased: {
                    buttonRectOne.color = JamiTheme.releaseColor
                }
                onEntered: {
                    buttonRectOne.color = JamiTheme.hoverColor
                }
                onExited: {
                    buttonRectOne.color = Qt.binding(function () {
                        return pageOne.down ? "white" : JamiTheme.releaseColor
                    })
                }
            }
        }
    }

    TabButton {
        id: pageTwo

        width: tabBar.width / 2 - tabButtonShrinkSize
        height: textInvElement.height + 10

        Rectangle {
            id: pendingRequestCountRect

            anchors.right: pageTwo.right
            anchors.rightMargin: 5
            anchors.bottom: pageTwo.bottom
            anchors.bottomMargin: pageTwo.height - pendingRequestCountRect.height / 2

            width: 14
            height: 14

            visible: pendingRequestCount > 0

            Text {
                id: pendingRequestCountText

                anchors.centerIn: pendingRequestCountRect

                text: pendingRequestCount > 9 ? "···" : pendingRequestCount
                color: "white"
            }

            radius: 30
            color: JamiTheme.notificationRed
        }

        background: Rectangle {
            id: buttonRectTwo

            radius: 10
            color: pageTwo.down ? "white" : JamiTheme.releaseColor
            border.color: JamiTheme.tabbarBorderColor

            Text {
                id: textInvElement

                anchors.centerIn: buttonRectTwo

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pointSize: JamiTheme.textFontSize

                text: qsTr("Invitation")
                opacity: enabled ? 1.0 : 0.3
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPressed: {
                    buttonRectTwo.color = JamiTheme.pressColor
                    ConversationsAdapter.setConversationFilter("PENDING")
                    contactSearchBar.setPlaceholderString(
                                JamiTheme.contactSearchBarPlaceHolderInivitionText)
                    pageTwo.down = true
                    pageOne.down = false
                }
                onReleased: {
                    buttonRectTwo.color = JamiTheme.releaseColor
                }
                onEntered: {
                    buttonRectTwo.color = JamiTheme.hoverColor
                }
                onExited: {
                    buttonRectTwo.color = Qt.binding(function () {
                        return pageTwo.down ? "white" : JamiTheme.releaseColor
                    })
                }
            }
        }
    }
}
