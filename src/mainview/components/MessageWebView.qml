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
import QtQuick.Layouts 1.14
import QtWebEngine 1.10
import QtWebChannel 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"

Rectangle {
    id: messageWebViewRect

    property int messageWebViewHeaderPreferredHeight: 64
    property string headerUserAliasLabelText: ""
    property string headerUserUserNameLabelText: ""

    signal needToGoBackToWelcomeView
    signal needToHideConversationInCall
    signal needToSendContactRequest

    signal sendMessageContentSaved(string arg)
    signal messagesCleared
    signal messagesLoaded


    function webViewRunJavaScript(arg) {
        messageWebView.runJavaScript(arg)
    }

    function setSendContactRequestButtonVisible(visible) {
        messageWebViewHeader.sendContactRequestButtonVisible = visible
    }

    function setMessagingHeaderButtonsVisible(visible) {
        messageWebViewHeader.toggleMessagingHeaderButtonsVisible(visible)
    }

    function resetMessagingHeaderBackButtonSource(reset) {
        messageWebViewHeader.resetBackToWelcomeViewButtonSource(reset)
    }

    JamiFileDialog {
        id: jamiFileDialog

        mode: JamiFileDialog.Mode.OpenFiles

        onAccepted: {
            var filePaths = jamiFileDialog.files
            for (var index = 0; index < filePaths.length; ++index) {
                var path = UtilsAdapter.getAbsPath(filePaths[index])
                MessagesAdapter.setNewMessagesContent(path)
            }
        }
    }

    MessageWebViewHeader {

        DropArea{
            anchors.fill: parent
            onDropped: {
                var path = UtilsAdapter.getAbsPath(drop.text.toString())
                MessagesAdapter.setNewMessagesContent(path)
            }
        }

        id: messageWebViewHeader

        anchors.top: messageWebViewRect.top
        anchors.left: messageWebViewRect.left

        width: messageWebViewRect.width
        height: messageWebViewHeaderPreferredHeight

        userAliasLabelText: headerUserAliasLabelText
        userUserNameLabelText: headerUserUserNameLabelText

        onBackToWelcomeViewButtonClicked: {
            MessagesAdapter.updateDraft()
            messageWebViewRect.needToGoBackToWelcomeView()
        }

        onNeedToHideConversationInCall: {
            messageWebViewRect.needToHideConversationInCall()
        }

        onSendContactRequestButtonClicked: {
            MessagesAdapter.sendContactRequest()
        }
    }

    QtObject {
        id: jsBridgeObject


        // ID, under which this object will be known at chatview.js side.
        WebChannel.id: "jsbridge"


        // Functions that are exposed, return code can be derived from js side
        // by setting callback function.
        function deleteInteraction(arg) {
            MessagesAdapter.deleteInteraction(arg)
        }

        function retryInteraction(arg) {
            MessagesAdapter.retryInteraction(arg)
        }

        function openFile(arg) {
            MessagesAdapter.openFile(arg)
        }

        function acceptFile(arg) {
            MessagesAdapter.acceptFile(arg)
        }

        function refuseFile(arg) {
            MessagesAdapter.refuseFile(arg)
        }

        function sendMessage(arg) {
            MessagesAdapter.sendMessage(arg)
        }

        function sendImage(arg) {
            MessagesAdapter.sendImage(arg)
        }

        function sendFile(arg) {
            MessagesAdapter.sendFile(arg)
        }

        function selectFile() {
            jamiFileDialog.open()
        }

        function acceptInvitation() {
            MessagesAdapter.acceptInvitation()
        }

        function refuseInvitation() {
            MessagesAdapter.refuseInvitation()
        }

        function blockConversation() {
            MessagesAdapter.blockConversation()
        }

        function emitMessagesCleared() {
            messageWebViewRect.messagesCleared()
        }

        function emitMessagesLoaded() {
            messageWebViewRect.messagesLoaded()
        }

        function emitPasteKeyDetected() {
            MessagesAdapter.pasteKeyDetected()
        }

        function openAudioRecorder(spikePosX, spikePosY) {
            recordBox.openRecorder(spikePosX, spikePosY, false)
        }

        function openVideoRecorder(spikePosX, spikePosY) {
            recordBox.openRecorder(spikePosX, spikePosY, true)
        }

        function saveSendMessageContent(arg) {
            messageWebViewRect.sendMessageContentSaved(arg)
        }

        function onComposing(isComposing) {
            MessagesAdapter.onComposing(isComposing)
        }
    }

    WebEngineView {

        id: messageWebView

        anchors.top: messageWebViewHeader.bottom
        anchors.topMargin: 1
        anchors.left: messageWebViewRect.left

        width: messageWebViewRect.width
        height: messageWebViewRect.height - messageWebViewHeaderPreferredHeight

        settings.javascriptEnabled: true
        settings.javascriptCanOpenWindows: true
        settings.fullScreenSupportEnabled: true
        settings.allowRunningInsecureContent: true
        settings.localContentCanAccessRemoteUrls: true
        settings.localContentCanAccessFileUrls: true
        settings.errorPageEnabled: false
        settings.pluginsEnabled: false
        settings.screenCaptureEnabled: false
        settings.linksIncludedInFocusChain: false
        settings.localStorageEnabled: false

        webChannel: messageWebViewChannel
        profile: messageWebViewProfile

        DropArea{
            anchors.fill: parent
            onDropped: {
                var path = UtilsAdapter.getAbsPath(drop.text.toString())
                MessagesAdapter.setNewMessagesContent(path)
            }
        }

        onNavigationRequested: {
            if(request.navigationType === WebEngineView.LinkClickedNavigation) {
                MessagesAdapter.openUrl(request.url)
                request.action = WebEngineView.IgnoreRequest
            }
        }

        onLoadingChanged: {
            if (loadRequest.status == WebEngineView.LoadSucceededStatus) {
                messageWebView.runJavaScript(UtilsAdapter.getStyleSheet(
                                                 "chatcss",
                                                 UtilsAdapter.qStringFromFile(
                                                     ":/chatview.css")))
                messageWebView.runJavaScript(UtilsAdapter.getStyleSheet(
                                                 "chatwin",
                                                 UtilsAdapter.qStringFromFile(
                                                     ":/chatview-windows.css")))

                messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                 ":/jed.js"))
                messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                 ":/linkify.js"))
                messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                 ":/linkify-html.js"))
                messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                 ":/linkify-string.js"))
                messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                 ":/qwebchannel.js"))
                messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                 ":/chatview.js"))
                messageWebView.runJavaScript("init_i18n();")
                messageWebView.runJavaScript("displayNavbar(false);")
            }
        }
        Component.onCompleted: {
            messageWebView.loadHtml(UtilsAdapter.qStringFromFile(
                                        ":/chatview.html"), ":/chatview.html")
            messageWebView.url = "qrc:/chatview.html"
        }
    }


    // Provide WebEngineProfile.
    WebEngineProfile {
        id: messageWebViewProfile

        cachePath: UtilsAdapter.getCachePath()
        persistentStoragePath: UtilsAdapter.getCachePath()
        persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies
        httpCacheType: WebEngineProfile.NoCache
        httpUserAgent: "jami-windows"
    }


    // Provide WebChannel by registering jsBridgeObject.
    WebChannel {
        id: messageWebViewChannel
        registeredObjects: [jsBridgeObject]
    }
}
