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
import net.jami.Models 1.0
import net.jami.Constants 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"

Rectangle {
    id: messagingHeaderRect

    property string userAliasLabelText
    property string userUserNameLabelText
    property string backToWelcomeViewButtonSource: "qrc:/images/icons/ic_arrow_back_24px.svg"
    property alias sendContactRequestButtonVisible: sendContactRequestButton.visible

    signal backClicked
    signal needToHideConversationInCall
    signal pluginSelector

    function resetBackToWelcomeViewButtonSource(reset) {
        backToWelcomeViewButtonSource = reset ? "qrc:/images/icons/ic_arrow_back_24px.svg" : "qrc:/images/icons/round-close-24px.svg"
    }

    function toggleMessagingHeaderButtonsVisible(visible) {
        startAAudioCallButton.visible = visible
        startAVideoCallButton.visible = visible
    }

    color: JamiTheme.secondaryBackgroundColor

    RowLayout {
        id: messagingHeaderRectRowLayout

        anchors.fill: parent

        PushButton {
            id: backToWelcomeViewButton

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.leftMargin: 16

            source: backToWelcomeViewButtonSource

            normalColor: JamiTheme.secondaryBackgroundColor
            imageColor: JamiTheme.chatviewButtonColor

            onClicked: {
                if (backToWelcomeViewButtonSource === "qrc:/images/icons/ic_arrow_back_24px.svg")
                    messagingHeaderRect.backClicked()
                else
                    messagingHeaderRect.needToHideConversationInCall()
            }
        }

        Rectangle {
            id: userNameOrIdRect

            Layout.alignment: Qt.AlignLeft | Qt.AlignTop

            // Width + margin.
            Layout.preferredWidth: messagingHeaderRect.width
                                   - backToWelcomeViewButton.width - buttonGroup.width - 45
            Layout.fillHeight: true
            Layout.topMargin: 7
            Layout.bottomMargin: 7
            Layout.leftMargin: 16

            color: JamiTheme.transparentColor

            ColumnLayout {
                id: userNameOrIdColumnLayout

                anchors.fill: parent

                spacing: 0

                ElidedTextLabel {
                    id: userAliasLabel

                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    font.pointSize: JamiTheme.textFontSize + 2

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    eText: userAliasLabelText
                    maxWidth: userNameOrIdRect.width
                }

                ElidedTextLabel {
                    id: userUserNameLabel

                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    visible: text !== ""
                    font.pointSize: JamiTheme.textFontSize
                    color: JamiTheme.faddedLastInteractionFontColor

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    eText: userUserNameLabelText
                    maxWidth: userNameOrIdRect.width
                }
            }
        }

        Rectangle {
            id: buttonGroup

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: childrenRect.width + 18
            Layout.preferredHeight: childrenRect.height
            Layout.rightMargin: 16

            color: "transparent"

            PushButton {
                id: startAAudioCallButton

                anchors.right: startAVideoCallButton.left
                anchors.rightMargin: 16
                anchors.verticalCenter: buttonGroup.verticalCenter

                source: "qrc:/images/icons/ic_phone_24px.svg"

                normalColor: JamiTheme.secondaryBackgroundColor
                imageColor: JamiTheme.chatviewButtonColor

                onClicked: {
                    MessagesAdapter.sendContactRequest()
                    CallAdapter.placeAudioOnlyCall()
                    communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
                }
            }

            PushButton {
                id: startAVideoCallButton

                anchors.right:  selectPluginButton.visible ? selectPluginButton.left :
                                   sendContactRequestButton.visible ?
                                   sendContactRequestButton.left :
                                   buttonGroup.right
                anchors.rightMargin: 16
                anchors.verticalCenter: buttonGroup.verticalCenter

                source: "qrc:/images/icons/videocam-24px.svg"

                normalColor: JamiTheme.secondaryBackgroundColor
                imageColor: JamiTheme.chatviewButtonColor

                onClicked: {
                    MessagesAdapter.sendContactRequest()
                    CallAdapter.placeCall()
                    communicationPageMessageWebView.setSendContactRequestButtonVisible(false)
                }
            }

            PushButton {
                id: selectPluginButton

                visible: UtilsAdapter.checkShowPluginsButton(false)

                Connections {
                    target: PluginAdapter
                    function onPluginHandlersUpdateStatus() {
                        selectPluginButton.visible = UtilsAdapter.checkShowPluginsButton(false)
                    }
                }

                anchors.right: sendContactRequestButton.visible ?
                                   sendContactRequestButton.left :
                                   buttonGroup.right
                anchors.rightMargin: 16
                anchors.verticalCenter: buttonGroup.verticalCenter

                source: "qrc:/images/icons/extension_24dp.svg"

                normalColor: JamiTheme.secondaryBackgroundColor
                imageColor: JamiTheme.chatviewButtonColor

                onClicked: pluginSelector()
            }

            PushButton {
                id: sendContactRequestButton

                anchors.right: buttonGroup.right
                anchors.rightMargin: 8
                anchors.verticalCenter: buttonGroup.verticalCenter

                width: sendContactRequestButton.visible ? preferredSize : 0
                source: "qrc:/images/icons/person_add-24px.svg"

                normalColor: JamiTheme.secondaryBackgroundColor
                imageColor: JamiTheme.chatviewButtonColor

                onClicked: {
                    MessagesAdapter.sendContactRequest()
                    visible = false
                }
            }
        }
    }

    CustomBorder {
        commonBorder: false
        lBorderwidth: 0
        rBorderwidth: 0
        tBorderwidth: 0
        bBorderwidth: 1
        borderColor: JamiTheme.tabbarBorderColor
    }
}
