
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
import "../settingsview"

Window {
    id: mainViewWindow

    property int minWidth: sidePanelViewStackPreferedWidth
    property int minHeight: aboutPopUpDialog.contentHeight

    property int mainViewWindowPreferedWidth: 650
    property int mainViewWindowPreferedHeight: 600
    property int sidePanelViewStackPreferedWidth: 250
    property int welcomePageGroupPreferedWidth: 250
    property int aboutPopUpPreferedWidth: 250

    property int savedSidePanelViewMinWidth: 0
    property int savedSidePanelViewMaxWidth: 0
    property int savedWelcomeViewMinWidth: 0
    property int savedWelcomeViewMaxWidth: 0
    property bool hiddenView: false

    /*
     * To calculate tab bar bottom border hidden rect left margin.
     */
    property int tabBarLeftMargin: 8
    property int tabButtonShrinkSize: 8

    signal noAccountIsAvailable
    signal needToAddNewAccount
    signal closeApp

    function newAccountAdded(index) {
        mainViewWindowSidePanel.refreshAccountComboBox(index)
    }

    function recursionStackViewItemMove(stackOne, stackTwo) {


        /*
         * Move all items (expect the bottom item) to stacktwo by the same order in stackone.
         */
        if (stackOne.depth === 1) {
            return
        }

        var tempItem = stackOne.pop(StackView.Immediate)
        recursionStackViewItemMove(stackOne, stackTwo)
        stackTwo.push(tempItem, StackView.Immediate)
    }

    title: "Jami"
    visible: true
    width: mainViewWindowPreferedWidth
    height: mainViewWindowPreferedHeight
    minimumWidth: minWidth
    minimumHeight: minHeight

    Connections {
        target: CallAdapter

        function onShowCallStack(accountId, convUid, forceReset) {
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
                if (welcomeViewStack.visible) {
                    welcomeViewStack.pop(null, StackView.Immediate)
                    welcomeViewStack.push(callStackView, StackView.Immediate)
                } else {
                    sidePanelViewStack.pop(null, StackView.Immediate)
                    sidePanelViewStack.push(callStackView, StackView.Immediate)
                }
            }
        }

        function onCloseCallStack(accountId, convUid) {

            var responsibleCallId = ClientWrapper.utilsAdaptor.getCallId(
                    callStackView.responsibleAccountId, callStackView.responsibleConvUid)
            var callId = ClientWrapper.utilsAdaptor.getCallId(
                    callStackView.responsibleAccountId, convUid)
            /*
             * Check if call stack view is on any of the stackview.
             */
            if (responsibleCallId === callId || responsibleCallId.length === 0) {
                if (welcomeViewStack.find(function (item, index) {
                    return item.objectName === "callStackViewObject"
                }) || sidePanelViewStack.find(function (item, index) {
                    return item.objectName === "callStackViewObject"
                })) {
                    callStackView.needToCloseInCallConversationAndPotentialWindow()
                    if (welcomeViewStack.visible) {
                        welcomeViewStack.pop(null, StackView.Immediate)
                        welcomeViewStack.push(communicationPageMessageWebView,
                                              StackView.Immediate)
                    } else {
                        sidePanelViewStack.pop(null, StackView.Immediate)
                        sidePanelViewStack.push(
                                    communicationPageMessageWebView,
                                    StackView.Immediate)
                    }
                }
            }
        }

        function onIncomingCallNeedToSetupMainView(accountId, convUid) {


            /*
             * Set up the call stack view that is needed by call overlay.
             */
            welcomeViewStack.pop(null, StackView.Immediate)
            sidePanelViewStack.pop(null, StackView.Immediate)

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
                implicitWidth: JamiTheme.splitViewHandlePreferedWidth
                implicitHeight: splitView.height
                color:"transparent"
                Rectangle {
                    implicitWidth: 1
                    implicitHeight: splitView.height
                    color: SplitHandle.pressed ? JamiTheme.pressColor : (SplitHandle.hovered ? JamiTheme.hoverColor : JamiTheme.tabbarBorderColor)
                }
            }

            StackView {
                id: sidePanelViewStack

                initialItem: mainViewWindowSidePanel

                SplitView.maximumWidth: splitView.width - sidePanelViewStackPreferedWidth
                SplitView.minimumWidth: sidePanelViewStackPreferedWidth
                SplitView.fillHeight: true

                clip: true
            }

            StackView {
                id: welcomeViewStack

                initialItem: welcomePage

                SplitView.maximumWidth: hiddenView ? splitView.width : splitView.width - sidePanelViewStackPreferedWidth
                SplitView.minimumWidth: sidePanelViewStackPreferedWidth
                SplitView.fillHeight: true

                clip: true
            }
        }

        SettingsView {
            id: settingsView

            Layout.fillWidth: true
            Layout.fillHeight: true

            onSettingsViewWindowNeedToShowMainViewWindow: {
                mainViewWindowSidePanel.refreshAccountComboBox(
                            accountDeleted ? 0 : -1)
                mainViewStackLayout.currentIndex = 0
            }

            onSettingsViewWindowNeedToShowNewWizardWindow: {
                mainViewWindow.noAccountIsAvailable()
            }
        }
    }

    AccountListModel {
        id: accountListModel
    }

    SidePanel {
        id: mainViewWindowSidePanel

        onSettingBtnClicked_AccountComboBox: {
            mainViewStackLayout.currentIndex = 1
        }

        onConversationSmartListNeedToAccessMessageWebView: {

            communicationPageMessageWebView.headerUserAliasLabelText = currentUserAlias
            communicationPageMessageWebView.headerUserUserNameLabelText = currentUserDisplayName

            callStackView.needToCloseInCallConversationAndPotentialWindow()
            callStackView.responsibleAccountId = ClientWrapper.utilsAdaptor.getCurrAccId()
            callStackView.responsibleConvUid = currentUID
            callStackView.updateCorrspondingUI()

            if (callStackViewShouldShow) {
                if (callStateStr == "Talking" || callStateStr == "Hold") {
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

            if (welcomeViewStack.find(function (item, index) {
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
            welcomeViewStack.pop(null, StackView.Immediate)
            sidePanelViewStack.pop(null, StackView.Immediate)

            if (sidePanelViewStack.visible && welcomeViewStack.visible) {
                if (callStackViewShouldShow) {
                    welcomeViewStack.push(callStackView)
                } else {
                    welcomeViewStack.push(communicationPageMessageWebView)
                }
            } else if (sidePanelViewStack.visible
                       && !welcomeViewStack.visible) {
                if (callStackViewShouldShow) {
                    sidePanelViewStack.push(callStackView)
                } else {
                    sidePanelViewStack.push(communicationPageMessageWebView)
                }
            } else if (!sidePanelViewStack.visible
                       && !welcomeViewStack.visible) {
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
            welcomeViewStack.pop(welcomePage)
            welcomePage.currentAccountIndex = index
            qrDialog.currentAccountIndex = index
        }

        onConversationSmartListViewNeedToShowWelcomePage: {
            welcomeViewStack.pop(welcomePage)
            welcomePage.currentAccountIndex = 0
            qrDialog.currentAccountIndex = 0
        }

        onAccountSignalsReconnect: {
            MessagesAdapter.accountChangedSetUp(accountId)
        }

        onNeedToUpdateConversationForAddedContact: {
            MessagesAdapter.updateConversationForAddedContact()
            mainViewWindowSidePanel.clearContactSearchBar()
            mainViewWindowSidePanel.forceReselectConversationSmartListCurrentIndex()
        }

        onNeedToAddNewAccount: {
            mainViewWindow.needToAddNewAccount()
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
                    && !welcomeViewStack.visible) {
                sidePanelViewStack.pop()
            } else if (communicationPageMessageWebView.visible
                       && welcomeViewStack.visible) {
                welcomeViewStack.pop()
            }
        }

        Component.onCompleted: {
            sidePanelViewStack.SplitView.maximumWidth = Qt.binding(function() {
                return (hiddenView ? splitView.width : splitView.width - sidePanelViewStackPreferedWidth)
            })

            recordBox.x = Qt.binding(function() {
                var i = (welcomeViewStack.width > 1000 ? Math.round((welcomeViewStack.width-1000)*0.5) : 0)
                return sidePanelViewStack.width + recordBox.x_offset + i
            })

            recordBox.y = Qt.binding(function() {
                return sidePanelViewStack.height + recordBox.y_offset
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
        if (mainViewWindow.width < sidePanelViewStackPreferedWidth
                + welcomePageGroupPreferedWidth - 5
                && welcomeViewStack.visible) {
            welcomeViewStack.visible = false
            hiddenView = true

            /*
             * The find callback function is called for each item in the stack.
             */
            var inWelcomeViewStack = welcomeViewStack.find(
                        function (item, index) {
                            return index > 0
                        })

            if (inWelcomeViewStack) {
                recursionStackViewItemMove(welcomeViewStack, sidePanelViewStack)
            }

            mainViewWindow.update()
        } else if (mainViewWindow.width >= sidePanelViewStackPreferedWidth
                   + welcomePageGroupPreferedWidth + 5
                   && !welcomeViewStack.visible) {
            welcomeViewStack.visible = true
            hiddenView = false

            var inSidePanelViewStack = sidePanelViewStack.find(
                        function (item, index) {
                            return index > 0
                        })
            if (inSidePanelViewStack) {
                recursionStackViewItemMove(sidePanelViewStack, welcomeViewStack)
            }

            mainViewWindow.update()
        }
    }

    AboutPopUp {
        id: aboutPopUpDialog

        x: Math.round((mainViewWindow.width - width) / 2)
        y: Math.round((mainViewWindow.height - height) / 2)
        width: Math.max(mainViewWindow.width / 2, aboutPopUpPreferedWidth)
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
        width: Math.max(mainViewWindow.width / 2, aboutPopUpPreferedWidth)
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
