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
import net.jami.Constants 1.0

import "../../commoncomponents"
import "../js/pluginhandlerpickercreation.js" as PluginHandlerPickerCreation

Rectangle {
    id: root

    property string headerUserAliasLabelText: ""
    property string headerUserUserNameLabelText: ""
    property bool jsLoaded: false

    signal needToHideConversationInCall

    signal sendMessageContentSaved(string arg)
    signal messagesCleared
    signal messagesLoaded

    function setSendMessageContent(content) {
        jsBridgeObject.setSendMessageContentRequest(content)
    }

    function focusMessageWebView() {
        messageWebView.forceActiveFocus()
    }

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

    function updateChatviewTheme() {
        var theme = 'setTheme("\
            --svg-invert-percentage:' + JamiTheme.invertPercentageInDecimal + ';\
            --jami-light-blue:' + JamiTheme.jamiLightBlue + ';\
            --jami-dark-blue: ' + JamiTheme.jamiDarkBlue + ';\
            --text-color: ' + JamiTheme.chatviewTextColor + ';\
            --timestamp-color:' + JamiTheme.timestampColor + ';\
            --message-out-bg:' + JamiTheme.messageOutBgColor + ';\
            --message-out-txt:' + JamiTheme.messageOutTxtColor + ';\
            --message-in-bg:' + JamiTheme.messageInBgColor + ';\
            --message-in-txt:' + JamiTheme.messageInTxtColor + ';\
            --file-in-timestamp-color:' + JamiTheme.fileOutTimestampColor + ';\
            --file-out-timestamp-color:' + JamiTheme.fileInTimestampColor + ';\
            --bg-color:' + JamiTheme.chatviewBgColor + ';\
            --action-icon-color:' + JamiTheme.chatviewButtonColor + ';\
            --action-icon-hover-color:' + JamiTheme.hoveredButtonColor + ';\
            --action-icon-press-color:' + JamiTheme.pressedButtonColor + ';\
            --placeholder-text-color:' + JamiTheme.placeholderTextColor + ';\
            --invite-hover-color:' + JamiTheme.inviteHoverColor + ';\
            --bg-text-input:' + JamiTheme.bgTextInput + ';\
            --bg-invitation-rect:' + JamiTheme.bgInvitationRectColor + ';\
            --preview-text-container-color:' + JamiTheme.previewTextContainerColor + ';\
            --preview-title-color:' + JamiTheme.previewTitleColor + ';\
            --preview-subtitle-color:' + JamiTheme.previewSubtitleColor + ';\
            --preview-image-background-color:' + JamiTheme.previewImageBackgroundColor + ';\
            --preview-card-container-color:' + JamiTheme.previewCardContainerColor + ';\
            --preview-url-color:' + JamiTheme.previewUrlColor + ';")'
        messageWebView.runJavaScript("init_picker(" + JamiTheme.darkTheme + ");")
        messageWebView.runJavaScript(theme);
    }

    color: JamiTheme.primaryBackgroundColor

    Connections {
        target: JamiTheme

        function onDarkThemeChanged() {
            updateChatviewTheme()
        }
    }

    QtObject {
        id: jsBridgeObject

        // ID, under which this object will be known at chatview.js side.
        WebChannel.id: "jsbridge"

        // signals to trigger functions in chatview.js
        // mainly used to avoid input arg string escape
        signal setSendMessageContentRequest(string content)

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
            root.messagesCleared()
        }

        function emitMessagesLoaded() {
            root.messagesLoaded()
        }

        function copyToDownloads(interactionId, displayName) {
            MessagesAdapter.copyToDownloads(interactionId, displayName)
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
            root.sendMessageContentSaved(arg)
        }

        function onComposing(isComposing) {
            MessagesAdapter.onComposing(isComposing)
        }

        function parseI18nData() {
            return MessagesAdapter.chatviewTranslatedStrings
        }

        function loadMessages(n) {
            return MessagesAdapter.loadMessages(n)
        }
    }

    // Provide WebChannel by registering jsBridgeObject.
    WebChannel {
        id: messageWebViewChannel
        registeredObjects: [jsBridgeObject]
    }

    ColumnLayout {
        anchors.fill: root

        spacing: 0

        MessageWebViewHeader {
            id: messageWebViewHeader

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.messageWebViewHeaderPreferredHeight
            Layout.maximumHeight: JamiTheme.messageWebViewHeaderPreferredHeight

            userAliasLabelText: headerUserAliasLabelText
            userUserNameLabelText: headerUserUserNameLabelText

            DropArea{
                anchors.fill: parent
                //onDropped: setFilePathsToSend(drop.urls)
            }

            onBackClicked: {
                MessagesAdapter.updateDraft()
                mainView.showWelcomeView()
            }

            onNeedToHideConversationInCall: {
                root.needToHideConversationInCall()
            }

            onPluginSelector : {
                // Create plugin handler picker - PLUGINS
                PluginHandlerPickerCreation.createPluginHandlerPickerObjects(root, false)
                PluginHandlerPickerCreation.calculateCurrentGeo(root.width / 2, root.height / 2)
                PluginHandlerPickerCreation.openPluginHandlerPicker()
            }
        }

        WebEngineView {
            id: messageWebView

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: messageWebViewHeader.hairLineSize
            Layout.bottomMargin: messageWebViewFooter.hairLineSize

            backgroundColor: "transparent"

            settings.javascriptEnabled: true
            settings.javascriptCanOpenWindows: true
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

            webChannel: messageWebViewChannel

            DropArea{
                anchors.fill: parent
                //onDropped: setFilePathsToSend(drop.urls)
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
                                                         ":/chatview-qt.css")))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/linkify.js"))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/linkify-html.js"))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/linkify-string.js"))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/qwebchannel.js"))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/jed.js"))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/emoji.js"))
                    messageWebView.runJavaScript(UtilsAdapter.qStringFromFile(
                                                     ":/previewInfo.js"))
                    messageWebView.runJavaScript(
                                UtilsAdapter.qStringFromFile(":/chatview.js"),
                                function() {
                                    messageWebView.runJavaScript("init_i18n();")
                                    MessagesAdapter.setDisplayLinks()
                                    updateChatviewTheme()
                                    messageWebView.runJavaScript("displayNavbar(false);")
                                    messageWebView.runJavaScript("hideMessageBar(true);")
                                    jsLoaded = true
                                })
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

                messageWebView.loadHtml(UtilsAdapter.qStringFromFile(
                                            ":/chatview.html"), ":/chatview.html")
                messageWebView.url = "qrc:/chatview.html"
            }
        }

        MessageWebViewFooter {
            id: messageWebViewFooter

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            Layout.maximumHeight: JamiTheme.messageWebViewFooterMaximumHeight
        }
    }
}
