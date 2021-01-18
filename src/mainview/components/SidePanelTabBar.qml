
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
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

TabBar {
    id: tabBar

    enum TabIndex {
        Conversations,
        Requests
    }

    Connections {
        target: ConversationsAdapter

        function onCurrentTypeFilterChanged() {
            pageOne.down = ConversationsAdapter.currentTypeFilter !==  Profile.Type.PENDING
            pageTwo.down = ConversationsAdapter.currentTypeFilter ===  Profile.Type.PENDING
            setCurrentUidSmartListModelIndex()
            forceReselectConversationSmartListCurrentIndex()
        }
    }

    function selectTab(tabIndex) {
        ConversationsAdapter.currentTypeFilter = tabIndex ===
                SidePanelTabBar.Conversations ? AccountAdapter.getCurrentAccountType() :
                                                Profile.Type.PENDING
    }

    property alias converstationTabWidth: pageOne.width
    property alias invitationTabWidth: pageTwo.width
    property alias converstationTabHeight: pageOne.height
    property alias invitationTabHeight: pageTwo.height
    property real opacityDegree: 0.5

    visible: tabBarVisible

    currentIndex: 0

    TabButton {

        id: pageOne
        down: true


        background: Rectangle {

            id: buttonRectOne
            width: tabBar.width / 2 + 1
            height: tabBar.height
            color: JamiTheme.backgroundColor

            Text {
                id: textConvElement

                anchors.horizontalCenter: buttonRectOne.horizontalCenter
                anchors.bottom: buttonRectOne.bottom
                anchors.bottomMargin: 12

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: JamiStrings.conversations
                font.pointSize: JamiTheme.textFontSize
                opacity: pageOne.down == true ? 1.0 : opacityDegree
                color: JamiTheme.textColor
            }

            Rectangle {
                id: totalUnreadMessagesCountRect

                anchors.left: textConvElement.right
                anchors.leftMargin: 4
                anchors.verticalCenter: textConvElement.verticalCenter
                anchors.verticalCenterOffset : -5

                width: 12
                height: 12

                visible: totalUnreadMessagesCount > 0

                Text {
                    id: totalUnreadMessagesCountText

                    anchors.centerIn: totalUnreadMessagesCountRect

                    text: totalUnreadMessagesCount > 9 ? "···" : totalUnreadMessagesCount
                    color: "white"
                    font.pointSize: JamiTheme.indicatorFontSize
                }
                radius: 30
                color: JamiTheme.notificationBlue
            }

            Rectangle {
                id: markerTabOne
                width: buttonRectOne.width
                anchors.bottom: buttonRectOne.bottom
                height: 2
                color: pageOne.down == true ? JamiTheme.textColor : "transparent"
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPressed: {
                    selectTab(SidePanelTabBar.Conversations)
                }
                onReleased: {
                    buttonRectOne.color = Qt.binding(function(){return JamiTheme.backgroundColor})
                }
                onEntered: {
                    buttonRectOne.color = Qt.binding(function(){return JamiTheme.hoverColor})
                }
                onExited: {
                    buttonRectOne.color = Qt.binding(function(){return JamiTheme.backgroundColor})
                }
            }

            Shortcut {
                sequence: "Ctrl+L"
                context: Qt.ApplicationShortcut
                enabled: buttonRectOne.visible
                onActivated: {
                    selectTab(SidePanelTabBar.Conversations)
                }
            }
        }
    }

    TabButton {

        id: pageTwo

        background: Rectangle {
            id: buttonRectTwo

            width: tabBar.width / 2
            height: tabBar.height
            color: JamiTheme.backgroundColor

            Text {
                id: textInvElement

                anchors.horizontalCenter: buttonRectTwo.horizontalCenter
                anchors.bottom: buttonRectTwo.bottom
                anchors.bottomMargin: 12

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pointSize: JamiTheme.textFontSize

                text: JamiStrings.invitations
                //opacity: enabled ? 1.0 : 0.3
                opacity: pageTwo.down == true ? 1.0 : opacityDegree
                color: JamiTheme.textColor
            }

            Rectangle {
                id: pendingRequestCountRect

                anchors.left: textInvElement.right
                anchors.leftMargin: 4
                anchors.verticalCenter: textInvElement.verticalCenter
                anchors.verticalCenterOffset : -5

                width: 12
                height: 12

                visible: pendingRequestCount > 0

                Text {
                    id: pendingRequestCountText

                    anchors.centerIn: pendingRequestCountRect

                    text: pendingRequestCount > 9 ? "···" : pendingRequestCount
                    color: "white"
                    font.pointSize: JamiTheme.indicatorFontSize
                }
                radius: 30
                color: JamiTheme.notificationBlue
            }

            Rectangle {
                id: markerTabTwo
                width: buttonRectTwo.width
                anchors.bottom: buttonRectTwo.bottom
                height: 2
                color: pageTwo.down == true ? JamiTheme.textColor : "transparent"
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPressed: {
                    selectTab(SidePanelTabBar.Requests)
                }
                onReleased: {
                    buttonRectTwo.color = Qt.binding(function(){return JamiTheme.backgroundColor})
                }
                onEntered: {
                    buttonRectTwo.color = Qt.binding(function(){return JamiTheme.hoverColor})
                }
                onExited: {
                    buttonRectTwo.color = Qt.binding(function(){return JamiTheme.backgroundColor})
                }
            }

            Shortcut {
                sequence: "Ctrl+R"
                context: Qt.ApplicationShortcut
                enabled: buttonRectTwo.visible
                onActivated: {
                    selectTab(SidePanelTabBar.Requests)
                }
            }
        }
    }
}
