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
import QtGraphicalEffects 1.12
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"

import "../../commoncomponents/js/contextmenugenerator.js" as ContextMenuGenerator

Item {
    id: root

    property string responsibleAccountId: ""
    property string responsibleConvUid: ""
    property int contactType: Profile.Type.INVALID

    function openMenu() {
        var hasCall = UtilsAdapter.getCallId(responsibleAccountId, responsibleConvUid) !== ""
        if (!hasCall) {
            ContextMenuGenerator.addMenuItem(qsTr("Start video call"),
                                             "qrc:/images/icons/videocam-24px.svg",
                                             function (){
                                                 ConversationsAdapter.selectConversation(
                                                             responsibleAccountId,
                                                             responsibleConvUid, false)
                                                 CallAdapter.placeCall()
                                             })
            ContextMenuGenerator.addMenuItem(qsTr("Start audio call"),
                                             "qrc:/images/icons/ic_phone_24px.svg",
                                             function (){
                                                 ConversationsAdapter.selectConversation(
                                                             responsibleAccountId,
                                                             responsibleConvUid, false)
                                                 CallAdapter.placeAudioOnlyCall()
                                             })

            ContextMenuGenerator.addMenuItem(qsTr("Clear conversation"),
                                             "qrc:/images/icons/ic_clear_24px.svg",
                                             function (){
                                                 UtilsAdapter.clearConversationHistory(
                                                             responsibleAccountId,
                                                             responsibleConvUid)
                                             })

            if (contactType === Profile.Type.RING || contactType === Profile.Type.SIP)  {
                ContextMenuGenerator.addMenuItem(qsTr("Remove contact"),
                                                 "qrc:/images/icons/round-remove_circle-24px.svg",
                                                 function (){
                                                     UtilsAdapter.removeConversation(
                                                                 responsibleAccountId,
                                                                 responsibleConvUid)
                                                 })
            }

        } else {
            ContextMenuGenerator.addMenuItem(JamiStrings.hangup,
                                             "qrc:/images/icons/ic_call_end_white_24px.svg",
                                             function (){
                                                 CallAdapter.hangUpACall(responsibleAccountId,
                                                                         responsibleConvUid)
                                             })
        }

        if ((contactType === Profile.Type.RING || contactType === Profile.Type.PENDING)) {
            if (contactType === Profile.Type.PENDING || !hasCall) {
                ContextMenuGenerator.addMenuSeparator()
            }

            if (contactType === Profile.Type.PENDING) {
                ContextMenuGenerator.addMenuItem(JamiStrings.acceptContactRequest,
                                                 "qrc:/images/icons/person_add-24px.svg",
                                                 function (){
                                                     MessagesAdapter.acceptInvitation(
                                                                 responsibleConvUid)
                                                 })
                ContextMenuGenerator.addMenuItem(JamiStrings.declineContactRequest,
                                                 "qrc:/images/icons/round-close-24px.svg",
                                                 function (){
                                                     MessagesAdapter.refuseInvitation(
                                                                 responsibleConvUid)
                                                 })
            }
            if (!hasCall) {
                ContextMenuGenerator.addMenuItem(qsTr("Block contact"),
                                                 "qrc:/images/icons/ic_block_24px.svg",
                                                 function (){
                                                     MessagesAdapter.blockConversation(
                                                                 responsibleConvUid)
                                                 })
            }
            ContextMenuGenerator.addMenuSeparator()
            ContextMenuGenerator.addMenuItem(qsTr("Profile"),
                                             "qrc:/images/icons/person-24px.svg",
                                             function (){
                                                 userProfile.open()
                                             })
        }

        root.height = ContextMenuGenerator.getMenu().height
        root.width = ContextMenuGenerator.getMenu().width
        ContextMenuGenerator.getMenu().open()
    }

    Component.onCompleted: {
        ContextMenuGenerator.createBaseContextMenuObjects(root)
    }
}
