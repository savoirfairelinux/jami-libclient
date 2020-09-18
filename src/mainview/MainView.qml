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
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0

// Import qml component files.
import "components"
import "../wizardview"
import "../settingsview"
import "../settingsview/components"

Window {
    id: mainViewWindow
    objectName: "mainViewWindow"

    property int minWidth: settingsViewPreferredWidth
    property int minHeight: 400

    property int mainViewWindowPreferredWidth: 650
    property int mainViewWindowPreferredHeight: 600
    property int sidePanelViewStackPreferredWidth: 250
    property int mainViewStackPreferredWidth: 250
    property int settingsViewPreferredWidth: 445
    property int onWidthChangedTriggerDistance: 5

    property int savedSidePanelViewMinWidth: 0
    property int savedSidePanelViewMaxWidth: 0
    property int savedWelcomeViewMinWidth: 0
    property int savedWelcomeViewMaxWidth: 0
    property bool sidePanelOnly: !mainViewStack.visible

    // To calculate tab bar bottom border hidden rect left margin.
    property int tabBarLeftMargin: 8
    property int tabButtonShrinkSize: 8
    property bool inSettingsView: false

    signal closeApp
    signal noAccountIsAvailable

    function showWelcomeView() {
        mainViewWindowSidePanel.deselectConversationSmartList()
        if (communicationPageMessageWebView.visible || callStackView.visible) {
            sidePanelViewStack.pop(StackView.Immediate)
            if (!sidePanelOnly) {
                mainViewStack.pop(welcomePage, StackView.Immediate)
            }
        }
    }

    function setCallStackView() {

        mainViewWindowSidePanel.deselectConversationSmartList()

        var currentAccount = AccountAdapter.currentAccountId
        var currentCallConv = UtilsAdapter.getCallConvForAccount(currentAccount)

        ConversationsAdapter.selectConversation(currentCallConv)
        var callId = UtilsAdapter.getCallId(currentAccount, currentCallConv)
        var callStatus = UtilsAdapter.getCallStatus(callId)

        switch (callStatus) {
        case Call.Status.INCOMING_RINGING:
            callStackView.showIncomingCallPage(currentAccount, currentCallConv)
            break
        case Call.Status.OUTGOING_RINGING:
            callStackView.showOutgoingCallPage()
            break
        default:
            if (UtilsAdapter.hasVideoCall()) {
                callStackView.showVideoCallPage(callId)
            } else {
                callStackView.showAudioCallPage()
            }
        }

        callStackView.responsibleAccountId = currentAccount
        callStackView.responsibleConvUid = currentCallConv
        callStackView.updateCorrespondingUI()
    }


    function pushCallStackView() {
        if (sidePanelOnly) {
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

    function newAccountAdded(index) {
        mainViewWindowSidePanel.refreshAccountComboBox(index)
        AccountAdapter.accountChanged(index)
    }

    function currentAccountIsCalling() {
        return UtilsAdapter.hasCall(AccountAdapter.currentAccountId)
    }

    function recursionStackViewItemMove(stackOne, stackTwo, depth=1) {
        // Move all items (expect the bottom item) to stacktwo by the same order in stackone.
        if (stackOne.depth === depth) {
            return
        }

        var tempItem = stackOne.pop(StackView.Immediate)
        recursionStackViewItemMove(stackOne, stackTwo, depth)
        stackTwo.push(tempItem, StackView.Immediate)
    }

    function toggleSettingsView() {
        if (!inSettingsView) {
            if (sidePanelOnly)
                sidePanelViewStack.push(leftPanelSettingsView, StackView.Immediate)
            else {
                mainViewStack.pop(welcomePage, StackView.Immediate)
                mainViewStack.push(settingsView, StackView.Immediate)
                sidePanelViewStack.push(leftPanelSettingsView, StackView.Immediate)

                var windowCurrentMinimizedSize = settingsViewPreferredWidth
                        + sidePanelViewStackPreferredWidth + onWidthChangedTriggerDistance
                if (mainViewWindow.width < windowCurrentMinimizedSize)
                    mainViewWindow.width = windowCurrentMinimizedSize
            }
        } else {
            sidePanelViewStack.pop(StackView.Immediate)
            mainViewStack.pop(StackView.Immediate)

            if (currentAccountIsCalling())
                setCallStackView()
            else
                mainViewWindowSidePanel.deselectConversationSmartList()
        }

        inSettingsView = !inSettingsView
    }

    title: "Jami"
    visible: true
    width: mainViewWindowPreferredWidth
    height: mainViewWindowPreferredHeight
    minimumWidth: minWidth
    minimumHeight: minHeight

    Connections {
        target: CallAdapter

        function onShowCallStack(accountId, convUid, forceReset) {

            if (forceReset) {
                callStackView.responsibleAccountId = accountId
                callStackView.responsibleConvUid = convUid
            }

            // Check if it is coming from the current responsible call,
            // and push views onto the correct stackview
            if (callStackView.responsibleAccountId === accountId
                    && callStackView.responsibleConvUid === convUid) {
                pushCallStackView()
            }
        }

        function onCloseCallStack(accountId, convUid) {
            // Check if call stack view is on any of the stackview.
            if (callStackView.responsibleAccountId === accountId
                    && callStackView.responsibleConvUid === convUid) {
                if (mainViewStack.find(function (item, index) {
                    return item.objectName === "callStackViewObject"
                }) || sidePanelViewStack.find(function (item, index) {
                    return item.objectName === "callStackViewObject"
                })) {
                    if (!inSettingsView) {
                        callStackView.needToCloseInCallConversationAndPotentialWindow()
                        pushCommunicationMessageWebView()
                    }
                }
            }
        }

        function onIncomingCallNeedToSetupMainView(accountId, convUid, fromNotification) {

            // Set up the call stack view that is needed by call overlay.
            if (!inSettingsView) {
                mainViewStack.pop(welcomePage, StackView.Immediate)
                sidePanelViewStack.pop(mainViewWindowSidePanel, StackView.Immediate)
            } else {
                toggleSettingsView()
            }

            var index = UtilsAdapter.getCurrAccList().indexOf(accountId)
            var name = UtilsAdapter.getBestName(accountId, convUid)
            var id = UtilsAdapter.getBestId(accountId, convUid)

            communicationPageMessageWebView.headerUserAliasLabelText = name
            communicationPageMessageWebView.headerUserUserNameLabelText = (name !== id) ? id : ""

            callStackView.needToCloseInCallConversationAndPotentialWindow()
            callStackView.setLinkedWebview(communicationPageMessageWebView)

            callStackView.responsibleAccountId = accountId
            callStackView.responsibleConvUid = convUid
            callStackView.updateCorrespondingUI()

            mainViewWindowSidePanel.refreshAccountComboBox(index)
            AccountAdapter.accountChanged(index)
            ConversationsAdapter.selectConversation(accountId, convUid, !fromNotification)
            MessagesAdapter.setupChatView(convUid)
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

            width: mainViewWindow.width
            height: mainViewWindow.height

            handle: Rectangle {
                implicitWidth: JamiTheme.splitViewHandlePreferredWidth
                implicitHeight: splitView.height
                color:"white"
                Rectangle {
                    implicitWidth: 1
                    implicitHeight: splitView.height
                    color: SplitHandle.pressed ? JamiTheme.pressColor :
                                                 (SplitHandle.hovered ? JamiTheme.hoverColor :
                                                                        JamiTheme.tabbarBorderColor)
                }
            }

            Rectangle {
                id: mainViewSidePanelRect

                SplitView.minimumWidth: sidePanelViewStackPreferredWidth
                SplitView.maximumWidth: (sidePanelOnly ? splitView.width :
                                                      splitView.width - sidePanelViewStackPreferredWidth)
                SplitView.fillHeight: true

                // AccountComboBox is always visible
                AccountComboBox {
                    id: accountComboBox

                    anchors.top: mainViewSidePanelRect.top
                    width: mainViewSidePanelRect.width
                    height: 64

                    visible: (mainViewWindowSidePanel.visible || leftPanelSettingsView.visible)

                    currentIndex: 0

                    Connections {
                        target: AccountAdapter

                        function onUpdateConversationForAddedContact() {
                            mainViewWindowSidePanel.needToUpdateConversationForAddedContact()
                        }

                        function onAccountStatusChanged() {
                            accountComboBox.resetAccountListModel()
                        }
                    }

                    onSettingBtnClicked: {
                        toggleSettingsView()
                    }

                    onAccountChanged: {
                        mainViewWindowSidePanel.deselectConversationSmartList()

                        mainViewWindowSidePanel.refreshAccountComboBox(index)
                        AccountAdapter.accountChanged(index)

                        settingsView.slotAccountListChanged()
                        settingsView.setSelected(settingsView.selectedMenu, true)

                        if (!inSettingsView) {
                            if (currentAccountIsCalling()) {
                                setCallStackView()
                            } else {
                                showWelcomeView()
                            }
                        }
                    }

                    onNeedToBackToWelcomePage: {
                        if (!inSettingsView && !currentAccountIsCalling()) {
                            mainViewWindowSidePanel.accountComboBoxNeedToShowWelcomePage()
                        }
                    }

                    onNewAccountButtonClicked: {
                        mainViewWindowSidePanel.needToAddNewAccount()
                    }

                    Component.onCompleted: {
                        AccountAdapter.setQmlObject(this)
                    }
                }

                StackView {
                    id: sidePanelViewStack

                    initialItem: mainViewWindowSidePanel

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
                                            splitView.width - sidePanelViewStackPreferredWidth
                SplitView.minimumWidth: sidePanelViewStackPreferredWidth
                SplitView.fillHeight: true

                clip: true
            }
        }

        WizardView {
            id: wizardView

            Layout.fillWidth: true
            Layout.fillHeight: true

            onNeedToShowMainViewWindow: {
                mainViewLoader.newAddedAccountIndex = accountIndex
                if (mainViewLoader.source.toString() !== "qrc:/src/mainview/MainView.qml") {
                    mainViewLoader.loaded.disconnect(slotNewAccountAdded)
                    mainViewLoader.loaded.connect(slotNewAccountAdded)
                    mainViewLoader.setSource("qrc:/src/mainview/MainView.qml")
                } else {
                    slotNewAccountAdded()
                }
                mainViewStackLayout.currentIndex = 0
            }

            onWizardViewIsClosed: {
                mainViewStackLayout.currentIndex = 0
            }
        }
    }

    AccountListModel {
        id: accountListModel
    }

    LeftPanelView {
        id: leftPanelSettingsView

        objectName: "leftPanelSettingsView"

        visible: false
        contentViewportWidth: mainViewSidePanelRect.width
        contentViewPortHeight: mainViewSidePanelRect.height

        Connections {
            target: leftPanelSettingsView.btnAccountSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.Account)
                if (sidePanelOnly)
                    sidePanelViewStack.push(settingsView, StackView.Immediate)
            }
        }
        Connections {
            target: leftPanelSettingsView.btnGeneralSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.General)
                if (sidePanelOnly)
                    sidePanelViewStack.push(settingsView, StackView.Immediate)
            }
        }
        Connections {
            target: leftPanelSettingsView.btnMediaSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.Media)
                if (sidePanelOnly)
                    sidePanelViewStack.push(settingsView, StackView.Immediate)
            }
        }
        Connections {
            target: leftPanelSettingsView.btnPluginSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.Plugin)
                if (sidePanelOnly)
                    sidePanelViewStack.push(settingsView, StackView.Immediate)
            }
        }
    }

    SidePanel {
        id: mainViewWindowSidePanel

        onConversationSmartListNeedToAccessMessageWebView: {

            communicationPageMessageWebView.headerUserAliasLabelText = currentUserAlias
            communicationPageMessageWebView.headerUserUserNameLabelText = currentUserDisplayName

            callStackView.needToCloseInCallConversationAndPotentialWindow()
            callStackView.responsibleAccountId = UtilsAdapter.getCurrAccId()
            callStackView.responsibleConvUid = currentUID
            callStackView.updateCorrespondingUI()

            if (callStackViewShouldShow) {
                if (callState === Call.Status.IN_PROGRESS || callState === Call.Status.PAUSED) {
                    UtilsAdapter.setCurrentCall(AccountAdapter.currentAccountId, currentUID)
                    if (isAudioOnly)
                        callStackView.showAudioCallPage()
                    else
                        callStackView.showVideoCallPage(
                                    UtilsAdapter.getCallId(
                                        callStackView.responsibleAccountId,
                                        callStackView.responsibleConvUid))
                } else if (callState === Call.Status.INCOMING_RINGING) {
                    callStackView.showIncomingCallPage(AccountAdapter.currentAccountId,
                                                       currentUID)
                } else {
                    callStackView.showOutgoingCallPage(callState)
                }
            }

            // Set up chatview.
            MessagesAdapter.setupChatView(currentUID)
            callStackView.setLinkedWebview(communicationPageMessageWebView)

            if (mainViewStack.find(function (item, index) {
                return item.objectName === "communicationPageMessageWebView"
            }) || sidePanelViewStack.find(function (item, index) {
                return item.objectName === "communicationPageMessageWebView"
            })) {
                if (!callStackViewShouldShow)
                    return
            }

            // Push messageWebView or callStackView onto the correct stackview
            mainViewStack.pop(welcomePage, StackView.Immediate)
            sidePanelViewStack.pop(mainViewWindowSidePanel, StackView.Immediate)

            if (sidePanelViewStack.visible && mainViewStack.visible) {
                if (callStackViewShouldShow) {
                    mainViewStack.push(callStackView)
                } else {
                    mainViewStack.push(communicationPageMessageWebView)
                }
            } else if (sidePanelViewStack.visible
                       && !mainViewStack.visible) {
                if (callStackViewShouldShow) {
                    sidePanelViewStack.push(callStackView)
                } else {
                    sidePanelViewStack.push(communicationPageMessageWebView)
                }
            } else if (!sidePanelViewStack.visible
                       && !mainViewStack.visible) {
                if (callStackViewShouldShow) {
                    sidePanelViewStack.push(callStackView)
                } else {
                    sidePanelViewStack.push(communicationPageMessageWebView)
                }
            }
        }

        onAccountComboBoxNeedToShowWelcomePage: {
            // If the item argument is specified, all items down to (but not including) item will be popped.
            if (!inSettingsView && !currentAccountIsCalling()) {
                showWelcomeView()
            }
        }

        onConversationSmartListViewNeedToShowWelcomePage: {
            showWelcomeView()
        }

        onNeedToUpdateConversationForAddedContact: {
            MessagesAdapter.updateConversationForAddedContact()
            mainViewWindowSidePanel.clearContactSearchBar()
            mainViewWindowSidePanel.forceReselectConversationSmartListCurrentIndex()
        }

        onNeedToAddNewAccount: {
            mainViewStackLayout.currentIndex = 1
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

        onSettingsViewWindowNeedToShowMainViewWindow: {
            mainViewWindowSidePanel.refreshAccountComboBox(0)
            AccountAdapter.accountChanged(0)
            toggleSettingsView()
        }

        onSettingsViewWindowNeedToShowNewWizardWindow: {
            mainViewWindow.noAccountIsAvailable()
        }

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
                mainViewWindowSidePanel.forceUpdateConversationSmartListView()
            }

            function onContactBanned() {
                showWelcomeView()
            }
        }

        onNeedToGoBackToWelcomeView: {
            showWelcomeView()
            recordBox.visible = false
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
        var widthToCompare = sidePanelViewStackPreferredWidth +
                (inSettingsView ? settingsViewPreferredWidth : mainViewStackPreferredWidth)

        if (mainViewWindow.width < widthToCompare - onWidthChangedTriggerDistance
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

            mainViewWindow.update()
        } else if (mainViewWindow.width >= widthToCompare + onWidthChangedTriggerDistance
                   && !mainViewStack.visible) {
            mainViewStack.visible = true

            var inSidePanelViewStack = sidePanelViewStack.find(
                        function (item, index) {
                            return index > 0
                        })

            if (inSettingsView) {
                if (sidePanelViewStack.currentItem.objectName !== leftPanelSettingsView.objectName)
                    sidePanelViewStack.pop(StackView.Immediate)
                mainViewStack.push(settingsView, StackView.Immediate)
            } else if (inSidePanelViewStack) {
                recursionStackViewItemMove(sidePanelViewStack, mainViewStack)
                if (currentAccountIsCalling())
                    pushCallStackView()
            }

            mainViewWindow.update()
        }
    }

    AboutPopUp {
        id: aboutPopUpDialog
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

    onClosing: {
        close.accepted = false
        mainViewWindow.hide()
        mainViewWindow.closeApp()
    }

    Shortcut {
        sequence: "Ctrl+M"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            leftPanelSettingsView.btnMediaSettings.clicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+G"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            leftPanelSettingsView.btnGeneralSettings.clicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+I"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            leftPanelSettingsView.btnAccountSettings.clicked()
        }
    }

    Shortcut {
        sequence: "Ctrl+P"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!inSettingsView) {
                toggleSettingsView()
            }
            leftPanelSettingsView.btnPluginSettings.clicked()
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
            if (mainViewWindow.visibility !== 5) // 5 = FullScreen
                mainViewWindow.visibility = "FullScreen"
            else
                mainViewWindow.visibility = "Windowed"
        }
    }

    Shortcut {
        sequence: "Ctrl+D"
        context: Qt.ApplicationShortcut
        onActivated: CallAdapter.hangUpThisCall()
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
        onActivated: mainViewWindowSidePanel.needToAddNewAccount()
    }

    KeyBoardShortcutTable {
        id: shortcutsTable
    }
}
