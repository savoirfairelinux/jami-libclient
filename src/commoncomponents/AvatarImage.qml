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

SpinningAnimation {
    id: root

    enum AvatarMode {
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
    property int avatarMode: AvatarImage.AvatarMode.FromAccount
    property string imageProviderIdPrefix: {
        switch (avatarMode) {
        case AvatarImage.AvatarMode.FromAccount:
            return "account_"
        case AvatarImage.AvatarMode.FromFile:
            return "file_"
        case AvatarImage.AvatarMode.FromContactUri:
            return "contact_"
        case AvatarImage.AvatarMode.FromConvUid:
            return "conversation_"
        case AvatarImage.AvatarMode.FromTemporaryName:
            return "fallback_"
        case AvatarImage.AvatarMode.FromBase64:
            return "base64_"
        case AvatarImage.AvatarMode.Default:
            return "default_"
        default:
            return ""
        }
    }

    // Full request url example: forceUpdateUrl_xxxxxxx_account_xxxxxxxx
    property string imageProviderUrl: "image://avatarImage/" + forceUpdateUrl
                                      + "_" + imageProviderIdPrefix
    property string imageId: ""
    property string forceUpdateUrl: Date.now()
    property alias presenceStatus: presenceIndicator.status
    property bool showPresenceIndicator: true
    property int unreadMessagesCount: 0
    property bool enableFadeAnimation: true

    signal imageIsReady

    function saveAvatarToConfig() {
        switch (avatarMode) {
        case AvatarImage.AvatarMode.FromFile:
            AccountAdapter.setCurrAccAvatar(true, imageId)
            break
        case AvatarImage.AvatarMode.FromBase64:
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
        var tempEnableAnimation = enableFadeAnimation
        var tempImageSource = rootImage.source

        enableFadeAnimation = false
        rootImage.source = ""

        enableFadeAnimation = tempEnableAnimation
        rootImage.source = tempImageSource
    }

    function rootImageOverlayReadyCallback() {
        if (rootImageOverlay.status === Image.Ready
                && (rootImageOverlay.state === "avatarImgFadeIn")) {
            rootImageOverlay.statusChanged.disconnect(
                        rootImageOverlayReadyCallback)

            rootImageOverlay.state = ''
        }
    }

    Item {
        id: imageGroup

        anchors.centerIn: root

        width: root.width - spinningAnimationWidth
        height: root.height - spinningAnimationWidth

        Image {
            id: rootImage

            anchors.fill: imageGroup

            smooth: true
            antialiasing: true
            asynchronous: true

            sourceSize.width: Math.max(24, width)
            sourceSize.height: Math.max(24, height)

            fillMode: Image.PreserveAspectFit

            onStatusChanged: {
                if (status === Image.Ready) {
                    if (enableFadeAnimation) {
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
                        duration: enableFadeAnimation ? 400 : 0
                    }

                    onRunningChanged: {
                        if ((rootImageOverlay.state === "avatarImgFadeIn")
                                && (!running)) {
                            if (rootImageOverlay.source === rootImage.source) {
                                rootImageOverlay.state = ''
                                return
                            }
                            rootImageOverlay.statusChanged.connect(
                                        rootImageOverlayReadyCallback)
                            rootImageOverlay.source = rootImage.source
                        }
                    }
                }
            }
        }

        PresenceIndicator {
            id: presenceIndicator

            anchors.right: imageGroup.right
            anchors.rightMargin: -1
            anchors.bottom: imageGroup.bottom
            anchors.bottomMargin: -1

            size: imageGroup.width * 0.26

            visible: showPresenceIndicator
        }

        Connections {
            target: ScreenInfo

            function onDevicePixelRatioChanged() {
                reloadImageSource()
            }
        }
    }
}
