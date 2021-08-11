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

import net.jami.Adapters 1.0
import net.jami.Constants 1.0

WebEngineView {
    id: root

    property string onCompletedLoadHtml: ""
    property string onCompletedUrl: "qrc" + onCompletedLoadHtml

    backgroundColor: "transparent"

    settings.javascriptEnabled: true
    settings.javascriptCanOpenWindows: false
    settings.javascriptCanAccessClipboard: true
    settings.javascriptCanPaste: true
    settings.fullScreenSupportEnabled: true
    settings.allowRunningInsecureContent: true
    settings.localContentCanAccessRemoteUrls: true
    settings.localContentCanAccessFileUrls: true
    settings.errorPageEnabled: false
    settings.pluginsEnabled: false
    settings.screenCaptureEnabled: false
    settings.linksIncludedInFocusChain: false
    settings.localStorageEnabled: true

    // Provide WebChannel by registering jsBridgeObject.
    webChannel: WebChannel {
        id: webViewChannel
    }

    onNavigationRequested: {
        if (request.navigationType === WebEngineView.LinkClickedNavigation) {
            MessagesAdapter.openUrl(request.url)
            request.action = WebEngineView.IgnoreRequest
        }
    }

    onContextMenuRequested: {
        var needContextMenu = request.selectedText.length || request.isContentEditable
        if (!needContextMenu)
            request.accepted = true
    }

    Component.onCompleted: {
        profile.cachePath = UtilsAdapter.getCachePath()
        profile.persistentStoragePath = UtilsAdapter.getCachePath()
        profile.persistentCookiesPolicy = WebEngineProfile.NoPersistentCookies
        profile.httpCacheType = WebEngineProfile.NoCache
        profile.httpUserAgent = JamiStrings.httpUserAgentName

        root.loadHtml(UtilsAdapter.qStringFromFile(onCompletedLoadHtml),
                      onCompletedLoadHtml)
        root.url = onCompletedUrl
    }
}