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
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

// Import qml component files.
import "components"
import "../"
import "../wizardview"
import "../settingsview"
import "../settingsview/components"

Rectangle {
    id: mainView

    objectName: "mainView"

    property var containerWindow: ""

    property int sidePanelViewStackMinimumWidth: 300
    property int mainViewStackPreferredWidth: 425
    property int settingsViewPreferredWidth: 460
    property int onWidthChangedTriggerDistance: 5

    property bool sidePanelOnly: (!mainViewStack.visible) && sidePanelViewStack.visible
    property int previousWidth: width

    // To calculate tab bar bottom border hidden rect left margin.
    property int tabBarLeftMargin: 8
    property int tabButtonShrinkSize: 8
    property bool inSettingsView: false

    // For updating msgWebView
    property string currentConvUID: ""

    signal loaderSourceChangeRequested(int sourceToLoad)

    property string currentAccountId: AccountAdapter.currentAccountId
    onCurrentAccountIdChanged: {
        var index = UtilsAdapter.getCurrAccList().indexOf(currentAccountId)
        mainViewSidePanel.refreshAccountComboBox(index)
        if (inSettingsView) {
            settingsView.accountListChanged()
            settingsView.setSelected(settingsView.selectedMenu, true)
        } else {
            backToMainView(true)
        }
    }

    function isPageInStack(objectName, stackView) {
        var foundItem = stackView.find(function (item, index) {
            return item.objectName === objectName
        })

        return foundItem ? true : false
    }

    function showWelcomeView() {
        currentConvUID = ""
        callStackView.needToCloseInCallConversationAndPotentialWindow()
        mainViewSidePanel.deselectConversationSmartList()
        if (isPageInStack("callStackViewObject", sidePanelViewStack) ||
                isPageInStack("communicationPageMessageWebView", sidePanelViewStack) ||
                isPageInStack("communicationPageMessageWebView", mainViewStack) ||
                isPageInStack("callStackViewObject", mainViewStack)) {
            sidePanelViewStack.pop(StackView.Immediate)
            mainViewStack.pop(welcomePage, StackView.Immediate)
        }
        recordBox.visible = false
    }

    function pushCallStackView() {
        if (sidePanelOnly) {
            sidePanelViewStack.pop(StackView.Immediate)
            sidePanelViewStack.push(callStackView, StackView.Immediate)
        } else {
            sidePanelViewStack.pop(StackView.Immediate)
            mainViewStack.pop(welcomePage, StackView.Immediate)
            mainViewStack.push(callStackView, StackView.Immediate)
        }
    }

    function pushCommunicationMessageWebView() {
        if (sidePanelOnly) {
            sidePanelViewStack.pop(StackView.Immediate)
            sidePanelViewStack.push(communicationPageMessageWebView, StackView.Immediate)
        } else {
            mainViewStack.pop(welcomePage, StackView.Immediate)
            mainViewStack.push(communicationPageMessageWebView, StackView.Immediate)
        }
    }

    function startWizard() {
        mainViewStackLayout.currentIndex = 1
    }

    function currentAccountIsCalling() {
        return UtilsAdapter.hasCall(AccountAdapter.currentAccountId)
    }

    // Only called onWidthChanged
    function recursionStackViewItemMove(stackOne, stackTwo, depth=1) {
        // Move all items (expect the bottom item) to stacktwo by the same order in stackone.
        if (stackOne.depth === depth) {
            return
        }

        var tempItem = stackOne.pop(StackView.Immediate)
        recursionStackViewItemMove(stackOne, stackTwo, depth)
        stackTwo.push(tempItem, StackView.Immediate)
    }

    // Back to WelcomeView required, but can also check, i. e., on account switch or
    // settings exit, if there is need to switch to a current call
    function backToMainView(checkCurrentCall = false) {
        if (inSettingsView)
            return
        if (checkCurrentCall && currentAccountIsCalling()) {
            var callConv = UtilsAdapter.getCallConvForAccount(
                        AccountAdapter.currentAccountId)
            ConversationsAdapter.selectConversation(
                        AccountAdapter.currentAccountId, callConv)
            CallAdapter.updateCall(callConv, currentAccountId)
        } else {
            showWelcomeView()
        }
    }

    function toggleSettingsView() {
        inSettingsView = !inSettingsView

        if (inSettingsView) {
            if (sidePanelOnly)
                sidePanelViewStack.push(settingsMenu, StackView.Immediate)
            else {
                mainViewStack.pop(welcomePage, StackView.Immediate)
                mainViewStack.push(settingsView, StackView.Immediate)
                sidePanelViewStack.push(settingsMenu, StackView.Immediate)

                var windowCurrentMinimizedSize = settingsViewPreferredWidth
                        + sidePanelViewStackMinimumWidth + onWidthChangedTriggerDistance
                if (containerWindow.width < windowCurrentMinimizedSize)
                    containerWindow.width = windowCurrentMinimizedSize
            }
        } else {
            sidePanelViewStack.pop(StackView.Immediate)
            mainViewStack.pop(StackView.Immediate)
            backToMainView(true)
        }
    }

    // ConversationSmartListViewItemDelegate provides UI information
    function setMainView(currentUserDisplayName, currentUserAlias, currentUID,
                               callStackViewShouldShow, isAudioOnly, callState) {
        if (!(communicationPageMessageWebView.jsLoaded)) {
            communicationPageMessageWebView.jsLoadedChanged.connect(
                        function(currentUserDisplayName, currentUserAlias, currentUID,
                                 callStackViewShouldShow, isAudioOnly, callState) {
                            return function() {
                                setMainView(currentUserDisplayName, currentUserAlias, currentUID,
                                            callStackViewShouldShow, isAudioOnly, callState)
                            }
                        }(currentUserDisplayName, currentUserAlias, currentUID,
                          callStackViewShouldShow, isAudioOnly, callState))
            return
        }

        if (callStackViewShouldShow) {
            if (inSettingsView) {
                toggleSettingsView()
            }
            MessagesAdapter.setupChatView(currentUID)
            communicationPageMessageWebView.headerUserAliasLabelText = currentUserAlias
            communicationPageMessageWebView.headerUserUserNameLabelText = currentUserDisplayName
            callStackView.setLinkedWebview(communicationPageMessageWebView)
            callStackView.responsibleAccountId = AccountAdapter.currentAccountId
            callStackView.responsibleConvUid = currentUID
            currentConvUID = currentUID

            if (callState === Call.Status.IN_PROGRESS || callState === Call.Status.PAUSED) {
                CallAdapter.updateCall(currentUID, AccountAdapter.currentAccountId)
                if (isAudioOnly)
                    callStackView.showAudioCallPage()
                else
                    callStackView.showVideoCallPage()
            } else if (callState === Call.Status.INCOMING_RINGING) {
                callStackView.showIncomingCallPage()
            } else {
                callStackView.showOutgoingCallPage(callState)
            }
            pushCallStackView()

        } else if (!inSettingsView) {
            if (currentConvUID !== currentUID) {
                callStackView.needToCloseInCallConversationAndPotentialWindow()
                MessagesAdapter.setupChatView(currentUID)
                communicationPageMessageWebView.headerUserAliasLabelText = currentUserAlias
                communicationPageMessageWebView.headerUserUserNameLabelText = currentUserDisplayName
                pushCommunicationMessageWebView()
                communicationPageMessageWebView.focusMessageWebView()
                currentConvUID = currentUID
            } else if (isPageInStack("callStackViewObject", sidePanelViewStack)
                       || isPageInStack("callStackViewObject", mainViewStack)) {
                callStackView.needToCloseInCallConversationAndPotentialWindow()
                pushCommunicationMessageWebView()
                communicationPageMessageWebView.focusMessageWebView()
            }
        }
    }

    color: JamiTheme.backgroundColor

    Connections {
        target: CallAdapter

        // selectConversation causes UI update
        function onCallSetupMainViewRequired(accountId, convUid) {
            ConversationsAdapter.selectConversation(accountId, convUid)
        }
    }

    Connections {
        target: JamiQmlUtils

        // TODO: call in fullscreen inside containerWindow
        function onCallIsFullscreenChanged() {
            if (JamiQmlUtils.callIsFullscreen) {
                UtilsAdapter.setSystemTrayIconVisible(false)
                containerWindow.hide()
            } else {
                UtilsAdapter.setSystemTrayIconVisible(true)
                containerWindow.show()
            }
        }
    }

    StackLayout {
        id: mainViewStackLayout

        anchors.fill: parent

        currentIndex: 0

        SplitView {
            id: splitView

            Layout.fillWidth: true
            Layout.fillHeight: true

            width: mainView.width
            height: mainView.height

            handle: Rectangle {
                implicitWidth: JamiTheme.splitViewHandlePreferredWidth
                implicitHeight: splitView.height
                color: JamiTheme.primaryBackgroundColor
                Rectangle {
                    implicitWidth: 1
                    implicitHeight: splitView.height
                    color: JamiTheme.tabbarBorderColor
                }
            }

            Rectangle {
                id: mainViewSidePanelRect

                SplitView.minimumWidth: sidePanelViewStackMinimumWidth
                SplitView.maximumWidth: (sidePanelOnly ?
                                             splitView.width :
                                             splitView.width - sidePanelViewStackMinimumWidth)
                SplitView.fillHeight: true
                color: JamiTheme.backgroundColor

                // AccountComboBox is always visible
                AccountComboBox {
                    id: accountComboBox

                    anchors.top: mainViewSidePanelRect.top
                    width: mainViewSidePanelRect.width
                    height: 64

                    visible: (mainViewSidePanel.visible || settingsMenu.visible)

                    currentIndex: 0

                    Connections {
                        target: AccountAdapter

                        function onUpdateConversationForAddedContact() {
                            MessagesAdapter.updateConversationForAddedContact()
                            mainViewSidePanel.clearContactSearchBar()
                            mainViewSidePanel.forceReselectConversationSmartListCurrentIndex()
                        }

                        function onAccountStatusChanged(accountId) {
                            accountComboBox.resetAccountListModel(accountId)
                        }
                    }

                    onSettingBtnClicked: {
                        toggleSettingsView()
                    }

                    Component.onCompleted: {
                        AccountAdapter.setQmlObject(this)
                    }
                }

                StackView {
                    id: sidePanelViewStack

                    initialItem: mainViewSidePanel

                    anchors.top: accountComboBox.visible ? accountComboBox.bottom :
                                                           mainViewSidePanelRect.top
                    width: mainViewSidePanelRect.width
                    height: accountComboBox.visible ? mainViewSidePanelRect.height - accountComboBox.height :
                                                      mainViewSidePanelRect.height

                    clip: true
                }
            }

            StackView {
                id: mainViewStack

                initialItem: welcomePage

                SplitView.maximumWidth: sidePanelOnly ?
                                            splitView.width :
                                            splitView.width - sidePanelViewStackMinimumWidth
                SplitView.preferredWidth: mainViewStackPreferredWidth
                SplitView.minimumWidth: sidePanelViewStackMinimumWidth
                SplitView.fillHeight: true

                clip: true
            }
        }

        WizardView {
            id: wizardView

            Layout.fillWidth: true
            Layout.fillHeight: true

            onLoaderSourceChangeRequested: {
                mainViewStackLayout.currentIndex = 0
                backToMainView()
            }

            onWizardViewIsClosed: {
                mainViewStackLayout.currentIndex = 0
                backToMainView()
            }
        }
    }

    AccountListModel {
        id: accountListModel

        lrcInstance: LRCInstance
    }

    SettingsMenu {
        id: settingsMenu

        objectName: "settingsMenu"

        visible: false

        width: mainViewSidePanelRect.width
        height: mainViewSidePanelRect.height

        onItemSelected: {
            settingsView.setSelected(index)
            if (sidePanelOnly)
                sidePanelViewStack.push(settingsView, StackView.Immediate)
        }
    }

    SidePanel {
        id: mainViewSidePanel

        Connections {
            target: ConversationsAdapter

            function onNavigateToWelcomePageRequested() {
                backToMainView()
            }
        }
    }

    CallStackView {
        id: callStackView

        visible: false
        objectName: "callStackViewObject"
    }

    WelcomePage {
        id: welcomePage

        visible: false
    }

    SettingsView {
        id: settingsView

        visible: false

        onSettingsViewNeedToShowMainView: {
            AccountAdapter.accountChanged(0)
            toggleSettingsView()
        }

        onSettingsViewNeedToShowNewWizardWindow: loaderSourceChangeRequested(
                                                     MainApplicationWindow.LoadedSource.WizardView)

        onSettingsBackArrowClicked: sidePanelViewStack.pop(StackView.Immediate)
    }

    MessageWebView {
        id: communicationPageMessageWebView

        objectName: "communicationPageMessageWebView"

        signal toSendMessageContentSaved(string arg)
        signal toMessagesCleared
        signal toMessagesLoaded

        visible: false

        Connections {
            target: MessagesAdapter

            function onNeedToUpdateSmartList() {
                mainViewSidePanel.forceUpdateConversationSmartListView()
            }

            function onNavigateToWelcomePageRequested() {
                backToMainView()
            }

            function onInvitationAccepted() {
                mainViewSidePanel.selectTab(SidePanelTabBar.Conversations)
                showWelcomeView()
            }
        }

        Component.onCompleted: {
            recordBox.x = Qt.binding(function() {
                var i = ((mainViewStack.visible && mainViewStack.width > 1000) ?
                             Math.round((mainViewStack.width-1000)*0.5) :
                             0)
                return mainViewStack.visible ?
                            sidePanelViewStack.width + recordBox.x_offset + i :
                            recordBox.x_offset + i

            })

            recordBox.y = Qt.binding(function() {
                return mainViewStack.visible ? mainViewStack.height + recordBox.y_offset :
                                               sidePanelViewStack.height + recordBox.y_offset
            })

            // Set qml MessageWebView object pointer to c++.
            MessagesAdapter.setQmlObject(this)
        }
    }

    onWidthChanged: {
        // Hide unnecessary stackview when width is changed.
        var widthToCompare = previousWidth < mainView.width ?
                    sidePanelViewStackMinimumWidth :
                    (sidePanelViewStackMinimumWidth +
                     (inSettingsView ? settingsViewPreferredWidth : mainViewStackPreferredWidth))

        if (mainView.width < widthToCompare - onWidthChangedTriggerDistance
                && mainViewStack.visible) {
            mainViewStack.visible = false

            // The find callback function is called for each item in the stack.
            var inWelcomeViewStack = mainViewStack.find(
                        function (item, index) {
                            return index > 0
                        })

            if (inSettingsView) {
                mainViewStack.pop(StackView.Immediate)
                sidePanelViewStack.push(settingsView, StackView.Immediate)
            }
            else if (inWelcomeViewStack)
                recursionStackViewItemMove(mainViewStack, sidePanelViewStack)
        } else if (mainView.width >= widthToCompare + onWidthChangedTriggerDistance
                   && !mainViewStack.visible) {
            mainViewStack.visible = true

            var inSidePanelViewStack = sidePanelViewStack.find(
                        function (item, index) {
                            return index > 0
                        })

            if (inSettingsView) {
                if (sidePanelViewStack.currentItem.objectName !== settingsMenu.objectName)
                    sidePanelViewStack.pop(StackView.Immediate)
                mainViewStack.push(settingsView, StackView.Immediate)
            } else if (inSidePanelViewStack) {
                recursionStackViewItemMove(sidePanelViewStack, mainViewStack)
                if (currentAccountIsCalling())
                    pushCallStackView()
            }
        }

        previousWidth = mainView.width
    }

    AboutPopUp {
        id: aboutPopUpDialog

        height: Math.min(preferredHeight,
                         mainView.height - JamiTheme.preferredMarginSize * 2)
    }

    WelcomePageQrDialog {
        id: qrDialog
    }

    RecordBox{
        id: recordBox
        visible: false
    }

    UserProfile {
        id: userProfile
    }

    Shortcut {
        sequence: "Ctrl+M"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            settingsMenu.btnMediaSettings.clicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+G"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            settingsMenu.btnGeneralSettings.clicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+I"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            settingsMenu.btnAccountSettings.clicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+P"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            settingsMenu.btnPluginSettings.clicked()
        }
    }

    Shortcut {
        sequence: "F10"
        context: Qt.ApplicationShortcut
        onActivated: {
            shortcutsTable.open()
        }
    }

    Shortcut {
        sequence: "F11"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (JamiQmlUtils.callIsFullscreen)
                return
            if (containerWindow.visibility !== Window.FullScreen)
                containerWindow.visibility = Window.FullScreen
            else
                containerWindow.visibility = Window.Windowed
        }
    }

    Shortcut {
        sequence: "Ctrl+D"
        context: Qt.ApplicationShortcut
        onActivated: CallAdapter.hangUpThisCall()
        onActivatedAmbiguously: CallAdapter.hangUpThisCall()
    }

    Shortcut {
        sequence: "Ctrl+Shift+A"
        context: Qt.ApplicationShortcut
        onActivated: {
            UtilsAdapter.makePermanentCurrentConv()
            communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+N"
        context: Qt.ApplicationShortcut
        onActivated: startWizard()
    }

    Shortcut {
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (JamiQmlUtils.callIsFullscreen)
                callStackView.toggleFullScreen()
        }
    }

    KeyBoardShortcutTable {
        id: shortcutsTable
    }
}
