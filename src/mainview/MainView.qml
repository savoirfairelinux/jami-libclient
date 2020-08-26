
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


/*
 * Import qml component files.
 */
import "components"
import "../wizardview"
import "../settingsview"
import "../settingsview/components"

Window {
    id: mainViewWindow

    property int minWidth: 400
    property int minHeight: aboutPopUpDialog.contentHeight

    property int mainViewWindowPreferredWidth: 650
    property int mainViewWindowPreferredHeight: 600
    property int sidePanelViewStackPreferredWidth: 250
    property int mainViewStackPreferredWidth: 250
    property int aboutPopUpPreferredWidth: 400

    property int savedSidePanelViewMinWidth: 0
    property int savedSidePanelViewMaxWidth: 0
    property int savedWelcomeViewMinWidth: 0
    property int savedWelcomeViewMaxWidth: 0
    property bool sidePanelHidden: false

    /*
     * To calculate tab bar bottom border hidden rect left margin.
     */
    property int tabBarLeftMargin: 8
    property int tabButtonShrinkSize: 8
    property bool inSettingsView: false
    property bool needToShowCallStack: false
    property bool needToCloseCallStack: false

    signal closeApp
    signal noAccountIsAvailable

    function pushCallStackView(){
        if (mainViewStack.visible) {
            mainViewStack.pop(null, StackView.Immediate)
            mainViewStack.push(callStackView, StackView.Immediate)
        } else {
            sidePanelViewStack.pop(null, StackView.Immediate)
            sidePanelViewStack.push(callStackView, StackView.Immediate)
        }
    }

    function pushCommunicationMessageWebView(){
        if (mainViewStack.visible) {
            mainViewStack.pop(null, StackView.Immediate)
            mainViewStack.push(communicationPageMessageWebView,
                                  StackView.Immediate)
        } else {
            sidePanelViewStack.pop(null, StackView.Immediate)
            sidePanelViewStack.push(
                        communicationPageMessageWebView,
                        StackView.Immediate)
        }
    }

    function newAccountAdded(index) {
        mainViewWindowSidePanel.refreshAccountComboBox(index)
    }

    function recursionStackViewItemMove(stackOne, stackTwo, depth=1) {

        /*
         * Move all items (expect the bottom item) to stacktwo by the same order in stackone.
         */
        if (stackOne.depth === depth) {
            return
        }

        var tempItem = stackOne.pop(StackView.Immediate)
        recursionStackViewItemMove(stackOne, stackTwo, depth)
        stackTwo.push(tempItem, StackView.Immediate)
    }

    function toggleSettingsView() {

        if (!inSettingsView) {

            if (sidePanelHidden){
                recursionStackViewItemMove(sidePanelViewStack, mainViewStack, 1)
                mainViewStack.push(settingsView, StackView.Immediate)
                sidePanelViewStack.push(leftPanelSettingsView, StackView.Immediate)
                recursionStackViewItemMove(mainViewStack, sidePanelViewStack, 1)
            } else {
                mainViewStack.push(settingsView, StackView.Immediate)
                sidePanelViewStack.push(leftPanelSettingsView, StackView.Immediate)
            }

        } else {

            if (!sidePanelHidden) {
                sidePanelViewStack.pop(mainViewWindowSidePanel, StackView.Immediate)
                mainViewStack.pop(StackView.Immediate)
            } else {
                recursionStackViewItemMove(sidePanelViewStack, mainViewStack, 2)
                sidePanelViewStack.pop(StackView.Immediate)
                mainViewStack.pop(StackView.Immediate)
                recursionStackViewItemMove(mainViewStack, sidePanelViewStack, 1)
            }

            if (needToCloseCallStack) {
                pushCommunicationMessageWebView()
                needToShowCallStack = false
                needToCloseCallStack = false
            }
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

            needToShowCallStack = true
            if (forceReset) {
                callStackView.responsibleAccountId = accountId
                callStackView.responsibleConvUid = convUid
            }


            /*
             * Check if it is coming from the current responsible call,
             * and push views onto the correct stackview
             */
            if (callStackView.responsibleAccountId === accountId
                    && callStackView.responsibleConvUid === convUid) {
                pushCallStackView()
            }
        }

        function onCloseCallStack(accountId, convUid) {

            /*
             * Check if call stack view is on any of the stackview.
             */
            if (callStackView.responsibleAccountId === accountId
                    && callStackView.responsibleConvUid === convUid) {
                if (mainViewStack.find(function (item, index) {
                    return item.objectName === "callStackViewObject"
                }) || sidePanelViewStack.find(function (item, index) {
                    return item.objectName === "callStackViewObject"
                }) || (inSettingsView && needToShowCallStack)) {
                    callStackView.needToCloseInCallConversationAndPotentialWindow()

                    if (!inSettingsView) {
                        pushCommunicationMessageWebView()
                        needToShowCallStack = false
                    } else {
                        needToCloseCallStack = true
                    }
                }
            }
        }

        function onIncomingCallNeedToSetupMainView(accountId, convUid) {

            /*
             * Set up the call stack view that is needed by call overlay.
             */
            if (!inSettingsView) {
                mainViewStack.pop(null, StackView.Immediate)
                sidePanelViewStack.pop(null, StackView.Immediate)
            } else {
                toggleSettingsView()
            }

            var index = ClientWrapper.utilsAdaptor.getCurrAccList().indexOf(accountId)
            var name = ClientWrapper.utilsAdaptor.getBestName(accountId, convUid)
            var id = ClientWrapper.utilsAdaptor.getBestId(accountId, convUid)

            communicationPageMessageWebView.headerUserAliasLabelText = name
            communicationPageMessageWebView.headerUserUserNameLabelText = (name !== id) ? id : ""

            callStackView.needToCloseInCallConversationAndPotentialWindow()
            callStackView.setLinkedWebview(
                        communicationPageMessageWebView)

            callStackView.responsibleAccountId = accountId
            callStackView.responsibleConvUid = convUid
            callStackView.updateCorrspondingUI()

            mainViewWindowSidePanel.needToChangeToAccount(accountId, index)
            ConversationsAdapter.selectConversation(accountId, convUid)

            MessagesAdapter.setupChatView(convUid)
        }
    }

    WizardView {
        id: wizardView

        anchors.fill: parent

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
                    color: SplitHandle.pressed ? JamiTheme.pressColor : (SplitHandle.hovered ? JamiTheme.hoverColor : JamiTheme.tabbarBorderColor)
                }
            }

            Rectangle {
                id: mainViewSidePanelRect
                SplitView.minimumWidth: sidePanelViewStackPreferredWidth
                SplitView.maximumWidth: (sidePanelHidden ? splitView.width :
                                                      splitView.width - sidePanelViewStackPreferredWidth)
                SplitView.fillHeight: true

                /*
                 * AccountComboBox is always visible
                 */
                AccountComboBox {
                    id: accountComboBox

                    anchors.top: mainViewSidePanelRect.top
                    width: mainViewSidePanelRect.width
                    height: 64

                    visible: (mainViewWindowSidePanel.visible || leftPanelSettingsView.visible)

                    currentIndex: 0

                    Connections {
                        target: ClientWrapper.accountAdaptor

                        function onUpdateConversationForAddedContact() {
                            mainViewWindowSidePanel.needToUpdateConversationForAddedContact()
                        }

                        function onAccountStatusChanged() {
                            accountComboBox.updateAccountListModel()
                        }
                    }

                    onSettingBtnClicked: {
                        toggleSettingsView()
                    }

                    onAccountChanged: {
                        mainViewWindowSidePanel.refreshAccountComboBox(index)
                        settingsView.slotAccountListChanged()
                        settingsView.setSelected(settingsView.selectedMenu, true)

                        if (needToShowCallStack
                                && callStackView.responsibleAccountId === ClientWrapper.utilsAdaptor.getCurrAccId()){
                            if (!ClientWrapper.accountAdaptor.hasVideoCall()) {
                                pushCommunicationMessageWebView()
                                needToShowCallStack = false
                            } else if (needToShowCallStack) {
                                pushCallStackView()
                            }
                        }
                    }

                    onNeedToBackToWelcomePage: {
                        if (!inSettingsView)
                            mainViewWindowSidePanel.accountComboBoxNeedToShowWelcomePage()
                    }

                    onNewAccountButtonClicked: {
                        mainViewWindowSidePanel.needToAddNewAccount()
                    }

                    Component.onCompleted: {
                        ClientWrapper.accountAdaptor.setQmlObject(this)
                    }
                }

                StackView {
                    id: sidePanelViewStack

                    initialItem: mainViewWindowSidePanel

                    anchors.top: accountComboBox.visible ? accountComboBox.bottom : mainViewSidePanelRect.top
                    width: mainViewSidePanelRect.width
                    height: accountComboBox.visible ? mainViewSidePanelRect.height - accountComboBox.height :
                                                      mainViewSidePanelRect.height

                    clip: true
                }
            }

            StackView {
                id: mainViewStack

                initialItem: welcomePage

                SplitView.maximumWidth: sidePanelHidden ? splitView.width : splitView.width - sidePanelViewStackPreferredWidth
                SplitView.minimumWidth: sidePanelViewStackPreferredWidth
                SplitView.fillHeight: true

                clip: true
            }
        }
    }

    AccountListModel {
        id: accountListModel
    }


    LeftPanelView {
        id: leftPanelSettingsView
        visible: false
        contentViewportWidth: mainViewSidePanelRect.width
        contentViewPortHeight: mainViewSidePanelRect.height

        Connections {
            target: leftPanelSettingsView.btnAccountSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.Account)
                if (sidePanelHidden) {
                    recursionStackViewItemMove(mainViewStack, sidePanelViewStack, 1)
                }
            }
        }
        Connections {
            target: leftPanelSettingsView.btnGeneralSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.General)
                if (sidePanelHidden) {
                    recursionStackViewItemMove(mainViewStack, sidePanelViewStack, 1)
                }
            }
        }
        Connections {
            target: leftPanelSettingsView.btnMediaSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.Media)
                if (sidePanelHidden) {
                    recursionStackViewItemMove(mainViewStack, sidePanelViewStack, 1)
                }
            }
        }
        Connections {
            target: leftPanelSettingsView.btnPluginSettings
            function onCheckedToggledForRightPanel(checked) {
                settingsView.setSelected(SettingsView.Plugin)
                if (sidePanelHidden) {
                    recursionStackViewItemMove(mainViewStack, sidePanelViewStack, 1)
                }
            }
        }
    }


    SidePanel {
        id: mainViewWindowSidePanel

        onConversationSmartListNeedToAccessMessageWebView: {

            communicationPageMessageWebView.headerUserAliasLabelText = currentUserAlias
            communicationPageMessageWebView.headerUserUserNameLabelText = currentUserDisplayName

            callStackView.needToCloseInCallConversationAndPotentialWindow()
            callStackView.responsibleAccountId = ClientWrapper.utilsAdaptor.getCurrAccId()
            callStackView.responsibleConvUid = currentUID
            callStackView.updateCorrspondingUI()

            if (callStackViewShouldShow) {
                if (callState === Call.Status.IN_PROGRESS || callState === Call.Status.PAUSED) {
                    ClientWrapper.utilsAdaptor.setCurrentCall(
                                ClientWrapper.utilsAdaptor.getCurrAccId(),
                                currentUID)
                    if (isAudioOnly)
                        callStackView.showAudioCallPage()
                    else
                        callStackView.showVideoCallPage(
                                    ClientWrapper.utilsAdaptor.getCallId(
                                        callStackView.responsibleAccountId,
                                        callStackView.responsibleConvUid))
                } else {
                    callStackView.showOutgoingCallPage(callStateStr)
                }
            }


            /*
             * Set up chatview.
             */
            MessagesAdapter.setupChatView(currentUID)
            callStackView.setLinkedWebview(
                        communicationPageMessageWebView)

            if (mainViewStack.find(function (item, index) {
                return item.objectName === "communicationPageMessageWebView"
            }) || sidePanelViewStack.find(function (item, index) {
                return item.objectName === "communicationPageMessageWebView"
            })) {
                if (!callStackViewShouldShow)
                    return
            }


            /*
             * Push messageWebView or callStackView onto the correct stackview
             */
            mainViewStack.pop(null, StackView.Immediate)
            sidePanelViewStack.pop(null, StackView.Immediate)

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

            /*
             * If the item argument is specified, all items down to (but not including) item will be popped.
             */
            if (!inSettingsView) {
                mainViewStack.pop(welcomePage)
                welcomePage.updateWelcomePage()
                qrDialog.updateQrDialog()
            }
        }

        onConversationSmartListViewNeedToShowWelcomePage: {
            mainViewStack.pop(welcomePage)
            welcomePage.updateWelcomePage()
            qrDialog.updateQrDialog()
        }

        onNeedToUpdateConversationForAddedContact: {
            MessagesAdapter.updateConversationForAddedContact()
            mainViewWindowSidePanel.clearContactSearchBar()
            mainViewWindowSidePanel.forceReselectConversationSmartListCurrentIndex()
        }

        onNeedToAddNewAccount: {
            mainViewStackLayout.currentIndex = 2
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

        width: Math.max(mainViewStackPreferredWidth, mainViewStack.width - 100)
        height: mainViewWindow.minimumHeight

        onSettingsViewWindowNeedToShowMainViewWindow: {
            mainViewWindowSidePanel.refreshAccountComboBox(
                        accountDeleted ? 0 : -1)
            toggleSettingsView()
        }

        onSettingsViewWindowNeedToShowNewWizardWindow: {
            mainViewWindow.noAccountIsAvailable()
        }

        onSettingsBackArrowClicked: {
            mainViewStack.pop(StackView.Immediate)
            recursionStackViewItemMove(sidePanelViewStack, mainViewStack, 2)
        }
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
        }

        onNeedToGoBackToWelcomeView: {
            mainViewWindowSidePanel.deselectConversationSmartList()
            if (communicationPageMessageWebView.visible
                    && !mainViewStack.visible) {
                sidePanelViewStack.pop()
            } else if (communicationPageMessageWebView.visible
                       && mainViewStack.visible) {
                mainViewStack.pop()
            }
            recordBox.visible = false
        }

        Component.onCompleted: {

            sidePanelViewStack.SplitView.maximumWidth = Qt.binding(function() {
                return (hiddenView ? splitView.width :
                                     splitView.width - sidePanelViewStackPreferedWidth)
            })

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


            /*
             * Set qml MessageWebView object pointer to c++.
             */
            MessagesAdapter.setQmlObject(this)
        }
    }

    onWidthChanged: {


        /*
         * Hide unnecessary stackview when width is changed.
         */
        if (mainViewWindow.width < sidePanelViewStackPreferredWidth
                + mainViewStackPreferredWidth - 5
                && mainViewStack.visible) {
            mainViewStack.visible = false
            sidePanelHidden = true

            /*
             * The find callback function is called for each item in the stack.
             */
            var inWelcomeViewStack = mainViewStack.find(
                        function (item, index) {
                            return index > 0
                        })

            if (inWelcomeViewStack) {
                recursionStackViewItemMove(mainViewStack, sidePanelViewStack)
            }

            mainViewWindow.update()
        } else if (mainViewWindow.width >= sidePanelViewStackPreferredWidth
                   + mainViewStackPreferredWidth + 5
                   && !mainViewStack.visible) {
            mainViewStack.visible = true
            sidePanelHidden = false

            var inSidePanelViewStack = sidePanelViewStack.find(
                        function (item, index) {
                            return index > 0
                        })
            if (inSidePanelViewStack) {
                recursionStackViewItemMove(sidePanelViewStack, mainViewStack, (inSettingsView ? 2 : 1))
            }

            mainViewWindow.update()
        }
    }

    AboutPopUp {
        id: aboutPopUpDialog

        x: Math.round((mainViewWindow.width - width) / 2)
        y: Math.round((mainViewWindow.height - height) / 2)
        width: aboutPopUpPreferredWidth
        height: aboutPopUpDialog.contentHeight
    }

    WelcomePageQrDialog {
        id: qrDialog

        x: Math.round((mainViewWindow.width - width) / 2)
        y: Math.round((mainViewWindow.height - height) / 2)
        width: qrDialog.contentHeight
        height: qrDialog.contentHeight
    }

    RecordBox{
        id: recordBox
        visible: false
    }

    UserProfile {
        id: userProfile

        x: Math.round((mainViewWindow.width - width) / 2)
        y: Math.round((mainViewWindow.height - height) / 2)
        width: Math.max(mainViewWindow.width / 2, aboutPopUpPreferredWidth)
        height: userProfile.contentHeight
    }

    Component.onCompleted: {
        CallAdapter.initQmlObject()
    }

    onClosing: {
        close.accepted = false
        mainViewWindow.hide()
        mainViewWindow.closeApp()
    }
}
