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
import QtWebEngine 1.10
import QtWebChannel 1.14

import net.jami.Constants 1.0
import net.jami.Adapters 1.0

import "../"

Rectangle {
    id: root

    signal emojiIsPicked(string content)

    function openEmojiPicker() {
        emojiPickerWebView.focus = true
        visible = true
        emojiPickerWebView.runJavaScript(
                    "prepare_to_show(" + JamiTheme.darkTheme + ");")
    }

    function closeEmojiPicker() {
        emojiPickerWebView.runJavaScript("prepare_to_hide();")
    }

    implicitWidth: 400
    implicitHeight: 425

    visible: false

    color: JamiTheme.transparentColor

    QtObject {
        id: jsBridgeObject

        // ID, under which this object will be known at chatview.js side.
        WebChannel.id: "jsbridge"

        // Functions that are exposed, return code can be derived from js side
        // by setting callback function.
        function emojiIsPicked(arg) {
            root.emojiIsPicked(arg)
        }

        // For emojiPicker to properly close
        function emojiPickerHideFinished() {
            root.visible = false
        }
    }

    WebEngineView {
        id: emojiPickerWebView

        anchors.fill: root

        backgroundColor: JamiTheme.transparentColor

        settings.javascriptEnabled: true
        settings.javascriptCanOpenWindows: false
        settings.javascriptCanAccessClipboard: true
        settings.javascriptCanPaste: true
        settings.fullScreenSupportEnabled: true
        settings.allowRunningInsecureContent: true
        settings.localContentCanAccessRemoteUrls: false
        settings.localContentCanAccessFileUrls: false
        settings.errorPageEnabled: false
        settings.pluginsEnabled: false
        settings.screenCaptureEnabled: false
        settings.linksIncludedInFocusChain: false
        settings.localStorageEnabled: true

        webChannel: emojiPickerWebViewChannel

        onLoadingChanged: {
            if (loadRequest.status == WebEngineView.LoadSucceededStatus) {
                emojiPickerWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":qwebchannel.js"))
                emojiPickerWebView.runJavaScript(
                            UtilsAdapter.qStringFromFile(
                                ":/src/commoncomponents/emojipicker/emoji.js"))
                emojiPickerWebView.runJavaScript(
                            UtilsAdapter.qStringFromFile(
                                ":/src/commoncomponents/emojipicker/emojiPickerLoader.js"))
                emojiPickerWebView.runJavaScript(
                            "init_emoji_picker(" + JamiTheme.darkTheme + ");")
            }
        }

        onActiveFocusChanged: {
            if (visible) {
                closeEmojiPicker()
            }
        }

        Component.onCompleted: {
            profile.cachePath = UtilsAdapter.getCachePath()
            profile.persistentStoragePath = UtilsAdapter.getCachePath()
            profile.persistentCookiesPolicy = WebEngineProfile.NoPersistentCookies
            profile.httpCacheType = WebEngineProfile.NoCache
            profile.httpUserAgent = JamiStrings.httpUserAgentName

            emojiPickerWebView.loadHtml(
                        UtilsAdapter.qStringFromFile(
                            ":/src/commoncomponents/emojipicker/emojiPickerLoader.html"),
                        ":/src/commoncomponents/emojipicker/emojiPickerLoader.html")
            emojiPickerWebView.url = "qrc:/src/commoncomponents/emojipicker/emojiPickerLoader.html"
        }
    }

    // Provide WebChannel by registering jsBridgeObject.
    WebChannel {
        id: emojiPickerWebViewChannel
        registeredObjects: [jsBridgeObject]
    }
}
