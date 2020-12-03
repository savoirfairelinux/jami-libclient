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
import QtQuick.Window 2.14
import net.jami.Constants 1.0

Item {
    id: root

    enum Mode {
        FromAccount = 0,
        FromFile,
        FromContactUri,
        FromConvUid,
        FromBase64,
        FromTemporaryName,
        Default
    }

    property alias fillMode: rootImage.fillMode
    property alias sourceSize: rootImage.sourceSize
    property bool saveToConfig: false
    property int mode: AvatarImage.Mode.FromAccount
    property string imageProviderIdPrefix: {
        switch(mode) {
        case AvatarImage.Mode.FromAccount:
            return "account_"
        case AvatarImage.Mode.FromFile:
            return "file_"
        case AvatarImage.Mode.FromContactUri:
            return "contact_"
        case AvatarImage.Mode.FromConvUid:
            return "conversation_"
        case AvatarImage.Mode.FromTemporaryName:
            return "fallback_"
        case AvatarImage.Mode.FromBase64:
            return "base64_"
        case AvatarImage.Mode.Default:
            return "default_"
        default:
            return ""
        }
    }

    // Full request url example: forceUpdateUrl_xxxxxxx_account_xxxxxxxx
    property string imageProviderUrl: "image://avatarImage/" + forceUpdateUrl + "_" +
                                      imageProviderIdPrefix
    property string imageId: ""
    property string forceUpdateUrl: Date.now()
    property alias presenceStatus: presenceIndicator.status
    property bool showPresenceIndicator: true
    property int unreadMessagesCount: 0
    property bool enableAnimation: true

    signal imageIsReady

    function saveAvatarToConfig() {
        switch(mode) {
        case AvatarImage.Mode.FromFile:
            AccountAdapter.setCurrAccAvatar(true, imageId)
            break
        case AvatarImage.Mode.FromBase64:
            AccountAdapter.setCurrAccAvatar(false, imageId)
            break
        default:
            return
        }
    }

    function updateImage(updatedId, oneTimeForceUpdateUrl) {
        imageId = updatedId
        if (oneTimeForceUpdateUrl === undefined)
            forceUpdateUrl = Date.now()
        else
            forceUpdateUrl = oneTimeForceUpdateUrl

        rootImage.source = imageProviderUrl + imageId

        if (saveToConfig)
            saveAvatarToConfig()
    }

    Image {
        id: rootImage

        anchors.fill: root

        smooth: false
        antialiasing: true
        asynchronous: true

        sourceSize.width: Math.max(24, width)
        sourceSize.height: Math.max(24, height)

        fillMode: Image.PreserveAspectFit

        onStatusChanged: {
            if (status === Image.Ready) {
                if (enableAnimation) {
                    rootImageOverlay.state = ""
                    rootImageOverlay.state = "rootImageLoading"
                } else {
                    rootImageOverlay.source = rootImage.source
                    root.imageIsReady()
                }
            }
        }

        Component.onCompleted: {
            if (imageId)
                return source = imageProviderUrl + imageId
            return source = ""
        }

        Image {
            id: rootImageOverlay

            anchors.fill: rootImage

            smooth: false
            antialiasing: true
            asynchronous: true

            sourceSize.width: Math.max(24, width)
            sourceSize.height: Math.max(24, height)

            fillMode: Image.PreserveAspectFit

            opacity: enableAnimation ? 1 : 0

            onOpacityChanged: {
                if (opacity === 0)
                    source = rootImage.source
            }

            onStatusChanged: {
                if (status === Image.Ready && opacity === 0) {
                    opacity = 1
                    root.imageIsReady()
                }
            }

            states: State {
                name: "rootImageLoading"
                PropertyChanges { target: rootImageOverlay; opacity: 0}
            }

            transitions: Transition {
                NumberAnimation {
                    properties: "opacity"
                    easing.type: Easing.InOutQuad
                    duration: 400
                }
            }
        }
    }

    PresenceIndicator {
        id: presenceIndicator

        anchors.right: root.right
        anchors.bottom: root.bottom

        size: root.width * 0.3

        visible: showPresenceIndicator
    }

    Rectangle {
        id: unreadMessageCountRect

        anchors.right: root.right
        anchors.top: root.top

        width: root.width * 0.3
        height: root.width * 0.3

        visible: unreadMessagesCount > 0

        Text {
            id: unreadMessageCounttext

            anchors.centerIn: unreadMessageCountRect

            text: unreadMessagesCount > 9 ? "…" : unreadMessagesCount
            color: "white"
            font.pointSize: JamiTheme.textFontSize - 2
        }

        radius: 30
        color: JamiTheme.notificationRed
    }
}
