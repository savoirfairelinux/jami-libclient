/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

// TODO:
// - totalUnreadMessagesCount and pendingRequestCount could be
//   properties of ConversationsAdapter
// - onCurrentTypeFilterChanged shouldn't need to update the smartlist
// - tabBarVisible could be factored out

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
        ConversationsAdapter.currentTypeFilter =
                (tabIndex === SidePanelTabBar.Conversations) ?
                    AccountAdapter.currentAccountType :
                    Profile.Type.PENDING
    }

    visible: tabBarVisible

    currentIndex: 0

    FilterTabButton {
        id: pageOne

        tabBar: parent
        down: true
        labelText: JamiStrings.conversations
        onSelected: selectTab(SidePanelTabBar.Conversations)
        badgeCount: totalUnreadMessagesCount
        acceleratorSequence: "Ctrl+L"
    }

    FilterTabButton {
        id: pageTwo

        tabBar: parent
        labelText: JamiStrings.invitations
        onSelected: selectTab(SidePanelTabBar.Requests)
        badgeCount: pendingRequestCount
        acceleratorSequence: "Ctrl+R"
    }
}
