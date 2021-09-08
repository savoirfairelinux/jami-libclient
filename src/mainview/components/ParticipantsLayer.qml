/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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

import QtQuick

Item {
    id: root

    property var participantOverlays: []
    property var participantComponent: Qt.createComponent("ParticipantOverlay.qml")

    // returns true if participant is not fully maximized
    function showMaximize(pX, pY, pW, pH) {
        // Hack: -1 offset added to avoid problems with odd sizes
        return (pX - distantRenderer.getXOffset() !== 0
                || pY - distantRenderer.getYOffset() !== 0
                || pW < (distantRenderer.width - distantRenderer.getXOffset() * 2 - 1)
                || pH < (distantRenderer.height - distantRenderer.getYOffset() * 2 - 1))
    }

    function update(infos) {
        // TODO: in the future the conference layout should be entirely managed by the client
        // Hack: truncate and ceil participant's overlay position and size to correct
        // when they are not exacts
        callOverlay.updateUI()
        var showMax = false
        var showMin = false

        var deletedUris = []
        var currentUris = []
        for (var p in participantOverlays) {
            if (participantOverlays[p]) {
                var participant = infos.find(e => e.uri === participantOverlays[p].uri);
                if (participant) {
                    // Update participant's information
                    var newX = Math.trunc(distantRenderer.getXOffset()
                                          + participant.x * distantRenderer.getScaledWidth())
                    var newY = Math.trunc(distantRenderer.getYOffset()
                                          + participant.y * distantRenderer.getScaledHeight())

                    var newWidth = Math.ceil(participant.w * distantRenderer.getScaledWidth())
                    var newHeight = Math.ceil(participant.h * distantRenderer.getScaledHeight())

                    var newVisible = participant.w !== 0 && participant.h !== 0
                    if (participantOverlays[p].x !== newX)
                        participantOverlays[p].x = newX
                    if (participantOverlays[p].y !== newY)
                        participantOverlays[p].y = newY
                    if (participantOverlays[p].width !== newWidth)
                        participantOverlays[p].width = newWidth
                    if (participantOverlays[p].height !== newHeight)
                        participantOverlays[p].height = newHeight
                    if (participantOverlays[p].visible !== newVisible)
                        participantOverlays[p].visible = newVisible

                    showMax = showMaximize(participantOverlays[p].x,
                                           participantOverlays[p].y,
                                           participantOverlays[p].width,
                                           participantOverlays[p].height)

                    participantOverlays[p].setMenu(participant.uri, participant.bestName,
                                                   participant.isLocal, participant.active, showMax)
                    if (participant.videoMuted)
                        participantOverlays[p].setAvatar(true, participant.uri, participant.isLocal)
                    else
                        participantOverlays[p].setAvatar(false)
                    currentUris.push(participantOverlays[p].uri)
                } else {
                    // Participant is no longer in conference
                    deletedUris.push(participantOverlays[p].uri)
                    participantOverlays[p].destroy()
                }
            }
        }
        participantOverlays = participantOverlays.filter(part => !deletedUris.includes(part.uri))

        if (infos.length === 0) { // Return to normal call
            for (var part in participantOverlays) {
                if (participantOverlays[part]) {
                    participantOverlays[part].destroy()
                }
            }
            participantOverlays = []
        } else {
            for (var infoVariant in infos) {
                // Only create overlay for new participants
                if (!currentUris.includes(infos[infoVariant].uri)) {
                    var hover = participantComponent.createObject(root, {
                                                                      x: Math.trunc(distantRenderer.getXOffset() + infos[infoVariant].x * distantRenderer.getScaledWidth()),
                                                                      y: Math.trunc(distantRenderer.getYOffset() + infos[infoVariant].y * distantRenderer.getScaledHeight()),
                                                                      width: Math.ceil(infos[infoVariant].w * distantRenderer.getScaledWidth()),
                                                                      height: Math.ceil(infos[infoVariant].h * distantRenderer.getScaledHeight()),
                                                                      visible: infos[infoVariant].w !== 0 && infos[infoVariant].h !== 0
                                                                  })
                    if (!hover) {
                        console.log("Error when creating the hover")
                        return
                    }

                    showMax = showMaximize(hover.x, hover.y, hover.width, hover.height)

                    hover.setMenu(infos[infoVariant].uri, infos[infoVariant].bestName,
                                  infos[infoVariant].isLocal, infos[infoVariant].active, showMax)
                    if (infos[infoVariant].videoMuted)
                        hover.setAvatar(true, infos[infoVariant].uri, infos[infoVariant].isLocal)
                    else
                        hover.setAvatar(false)
                    participantOverlays.push(hover)
                }
            }
        }
    }
}
