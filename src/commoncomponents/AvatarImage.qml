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

    // FromUrl here is for grabToImage image url
    enum Mode {
        FromAccount = 0,
        FromFile,
        FromContactUri,
        FromConvUid,
        FromUrl,
        FromTemporaryName,
        Default
    }

    property alias fillMode: rootImage.fillMode
    property alias sourceSize: rootImage.sourceSize
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

    signal imageIsReady

    function updateImage(updatedId, oneTimeForceUpdateUrl) {
        imageId = updatedId
        if (oneTimeForceUpdateUrl === undefined)
            forceUpdateUrl = Date.now()
        else
            forceUpdateUrl = oneTimeForceUpdateUrl

        if (mode === AvatarImage.Mode.FromUrl)
            rootImage.source = imageId
        else
            rootImage.source = imageProviderUrl + imageId
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
                rootImageOverlay.state = ""
                rootImageOverlay.state = "rootImageLoading"
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
                NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad; duration: 400}
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
