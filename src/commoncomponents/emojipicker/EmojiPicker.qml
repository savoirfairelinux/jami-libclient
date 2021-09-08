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

import QtQuick
import QtQuick.Controls
import QtWebEngine
import QtWebChannel

import net.jami.Constants 1.1
import net.jami.Adapters 1.1

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

    GeneralWebEngineView {
        id: emojiPickerWebView

        anchors.fill: root

        webChannel.registeredObjects: [jsBridgeObject]

        onCompletedLoadHtml: ":/src/commoncomponents/emojipicker/emojiPickerLoader.html"

        onActiveFocusChanged: {
            if (visible) {
                closeEmojiPicker()
            }
        }

        onLoadingChanged: function (loadingInfo) {
            if (loadingInfo.status === WebEngineView.LoadSucceededStatus) {
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
    }
}
