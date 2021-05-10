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

import net.jami.Adapters 1.0
import net.jami.Constants 1.0
import net.jami.Models 1.0

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
    property int transitionDuration: 150
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

    function reloadImageSource() {
        var tempEnableAnimation = enableAnimation
        var tempImageSource = rootImage.source

        enableAnimation = false
        rootImage.source = ""

        enableAnimation = tempEnableAnimation
        rootImage.source = tempImageSource
    }

    function rootImageOverlayReadyCallback() {
        if (rootImageOverlay.status === Image.Ready &&
                (rootImageOverlay.state === "avatarImgFadeIn")) {
            rootImageOverlay.statusChanged.disconnect(rootImageOverlayReadyCallback)

            rootImageOverlay.state = ''
        }
    }

    Image {
        id: rootImage

        anchors.fill: root

        smooth: true
        antialiasing: true
        asynchronous: true

        sourceSize.width: Math.max(24, width)
        sourceSize.height: Math.max(24, height)

        fillMode: Image.PreserveAspectFit

        onStatusChanged: {
            if (status === Image.Ready) {
                if (enableAnimation) {
                    rootImageOverlay.state = "avatarImgFadeIn"
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

            smooth: true
            antialiasing: true
            asynchronous: true

            sourceSize.width: Math.max(24, width)
            sourceSize.height: Math.max(24, height)

            fillMode: Image.PreserveAspectFit

            states: State {
                name: "avatarImgFadeIn"
                PropertyChanges {
                    target: rootImageOverlay
                    opacity: 0
                }
            }

            transitions: Transition {
                NumberAnimation {
                    properties: "opacity"
                    easing.type: Easing.InOutQuad
                    duration: enableAnimation ? 400 : 0
                }

                onRunningChanged: {
                    if ((rootImageOverlay.state === "avatarImgFadeIn") && (!running)) {
                        if (rootImageOverlay.source === rootImage.source) {
                            rootImageOverlay.state = ''
                            return
                        }
                        rootImageOverlay.statusChanged.connect(rootImageOverlayReadyCallback)
                        rootImageOverlay.source = rootImage.source
                    }
                }
            }
        }
    }

    PresenceIndicator {
        id: presenceIndicator

        anchors.right: root.right
        anchors.rightMargin: -1
        anchors.bottom: root.bottom
        anchors.bottomMargin: -1

        size: root.width * 0.26

        visible: showPresenceIndicator
    }

    Connections {
        target: ScreenInfo

        function onDevicePixelRatioChanged(){
            reloadImageSource()
        }
    }
}
