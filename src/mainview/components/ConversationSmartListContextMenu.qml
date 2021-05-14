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
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"
import "../../commoncomponents/contextmenu"

ContextMenuAutoLoader {
    id: root

    property string responsibleAccountId: ""
    property string responsibleConvUid: ""
    property bool isSwarm: false
    property int contactType: Profile.Type.INVALID
    property bool hasCall: {
        if (responsibleAccountId && responsibleConvUid)
            return UtilsAdapter.getCallId(responsibleAccountId,
                                          responsibleConvUid) !== ""
        return false
    }

    property list<GeneralMenuItem> menuItems: [
        GeneralMenuItem {
            id: startVideoCallItem

            canTrigger: !hasCall
            itemName: JamiStrings.startVideoCall
            iconSource: "qrc:/images/icons/videocam-24px.svg"
            onClicked: {
                LRCInstance.selectConversation(responsibleConvUid,
                                               responsibleAccountId)
                CallAdapter.placeCall()
                communicationPageMessageWebView.setSendContactRequestButtonVisible(
                            false)
            }
        },
        GeneralMenuItem {
            id: clearConversation

            canTrigger: !isSwarm
            itemName: JamiStrings.clearConversation
            iconSource: "qrc:/images/icons/place_audiocall-24px.svg"
            onClicked: {
                MessagesAdapter.clearConversationHistory(
                            responsibleAccountId,
                            responsibleConvUid)
            }
        },
        GeneralMenuItem {
            id: startAudioCall

            canTrigger: !hasCall
            itemName: JamiStrings.startAudioCall
            iconSource: "qrc:/images/icons/place_audiocall-24px.svg"
            onClicked: {
                LRCInstance.selectConversation(responsibleConvUid,
                                               responsibleAccountId)
                CallAdapter.placeAudioOnlyCall()
                communicationPageMessageWebView.setSendContactRequestButtonVisible(
                            false)
            }
        },
        GeneralMenuItem {
            id: clearConversation

            canTrigger: !hasCall
            itemName: JamiStrings.clearConversation
            iconSource: "qrc:/images/icons/ic_clear_24px.svg"
            onClicked: {
                MessagesAdapter.clearConversationHistory(responsibleAccountId,
                                                         responsibleConvUid)
            }
        },
        GeneralMenuItem {
            id: removeContact

            canTrigger: !hasCall && (contactType === Profile.Type.RING
                                     || contactType === Profile.Type.SIP)
            itemName: JamiStrings.removeContact
            iconSource: "qrc:/images/icons/ic_hangup_participant-24px.svg"
            onClicked: {
                MessagesAdapter.removeConversation(responsibleAccountId,
                                                   responsibleConvUid)
            }
        },
        GeneralMenuItem {
            id: hangup

            canTrigger: hasCall
            itemName: JamiStrings.hangup
            iconSource: "qrc:/images/icons/ic_call_end_white_24px.svg"
            addMenuSeparatorAfter: contactType !== Profile.Type.SIP
                                   && (contactType === Profile.Type.PENDING
                                       || !hasCall)
            onClicked: {
                CallAdapter.hangUpACall(responsibleAccountId,
                                        responsibleConvUid)
            }
        },
        GeneralMenuItem {
            id: acceptContactRequest

            canTrigger: contactType === Profile.Type.PENDING
            itemName: JamiStrings.acceptContactRequest
            iconSource: "qrc:/images/icons/add_people-24px.svg"
            onClicked: {
                MessagesAdapter.acceptInvitation(responsibleConvUid)
                communicationPageMessageWebView.setSendContactRequestButtonVisible(
                            false)
            }
        },
        GeneralMenuItem {
            id: declineContactRequest

            canTrigger: contactType === Profile.Type.PENDING
            itemName: JamiStrings.declineContactRequest
            iconSource: "qrc:/images/icons/round-close-24px.svg"
            onClicked: {
                MessagesAdapter.refuseInvitation(responsibleConvUid)
            }
        },
        GeneralMenuItem {
            id: blockContact

            canTrigger: !hasCall && contactType !== Profile.Type.SIP
            itemName: JamiStrings.blockContact
            iconSource: "qrc:/images/icons/ic_block_24px.svg"
            addMenuSeparatorAfter: contactType !== Profile.Type.SIP
            onClicked: {
                MessagesAdapter.blockConversation(responsibleConvUid)
            }
        },
        GeneralMenuItem {
            id: contactDetails

            canTrigger: contactType !== Profile.Type.SIP
            itemName: JamiStrings.contactDetails
            iconSource: "qrc:/images/icons/person-24px.svg"
            onClicked: {
                userProfile.open()
            }
        }
    ]

    Component.onCompleted: menuItemsToLoad = menuItems
}
