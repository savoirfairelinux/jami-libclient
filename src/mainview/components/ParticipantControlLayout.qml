/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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

import QtQuick.Layouts

import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

RowLayout {
    id: root

    property int visibleButtons: toggleModerator.visible
                                 + toggleMute.visible
                                 + maximizeParticipant.visible
                                 + minimizeParticipant.visible
                                 + hangupParticipant.visible

    ParticipantOverlayButton {
        id: toggleModerator

        visible: showSetModerator || showUnsetModerator
        preferredSize: iconButtonPreferredSize
        Layout.preferredHeight: buttonPreferredSize
        Layout.preferredWidth: buttonPreferredSize
        source: JamiResources.moderator_svg
        onClicked: CallAdapter.setModerator(uri, showSetModerator)
        toolTipText: showSetModerator? JamiStrings.setModerator
                                     : JamiStrings.unsetModerator
    }

    ParticipantOverlayButton {
        id: toggleMute

        visible: showModeratorMute || showModeratorUnmute
        preferredSize: iconButtonPreferredSize
        Layout.preferredHeight: buttonPreferredSize
        Layout.preferredWidth: buttonPreferredSize
        source: showModeratorMute ?
                    JamiResources.micro_black_24dp_svg :
                    JamiResources.micro_off_black_24dp_svg
        checkable: meModerator
        onClicked: {
            if (participantIsModeratorMuted && isLocalMuted) {
                if (isMe)
                    muteAlertMessage = JamiStrings.mutedLocally
                else
                    muteAlertMessage = JamiStrings.participantMicIsStillMuted
                muteAlertActive = true
            }
            CallAdapter.muteParticipant(uri, showModeratorMute)
        }
        toolTipText: {
            if (!checkable && participantIsModeratorMuted)
                return JamiStrings.mutedByModerator
            if (showModeratorMute)
                return JamiStrings.muteParticipant
            else
                return JamiStrings.unmuteParticipant
        }
    }

    ParticipantOverlayButton {
        id: maximizeParticipant

        visible: showMaximize
        preferredSize: iconButtonPreferredSize
        Layout.preferredHeight: buttonPreferredSize
        Layout.preferredWidth: buttonPreferredSize
        source: JamiResources.open_in_full_24dp_svg
        onClicked: CallAdapter.maximizeParticipant(uri)
        toolTipText: JamiStrings.maximizeParticipant
    }

    ParticipantOverlayButton {
        id: minimizeParticipant

        visible: showMinimize
        preferredSize: iconButtonPreferredSize
        Layout.preferredHeight: buttonPreferredSize
        Layout.preferredWidth: buttonPreferredSize
        source: JamiResources.close_fullscreen_24dp_svg
        onClicked: CallAdapter.minimizeParticipant(uri)
        toolTipText: JamiStrings.minimizeParticipant
    }

    ParticipantOverlayButton {
        id: hangupParticipant

        visible: showHangup
        preferredSize: iconButtonPreferredSize
        Layout.preferredHeight: buttonPreferredSize
        Layout.preferredWidth: buttonPreferredSize
        source: JamiResources.ic_hangup_participant_24dp_svg
        onClicked: CallAdapter.hangupParticipant(uri)
        toolTipText: JamiStrings.hangupParticipant
    }
}
