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

// JamiTheme as a singleton is to provide global theme property entry.
pragma Singleton

import QtQuick 2.15

import net.jami.Adapters 1.1
import net.jami.Enums 1.1

Item {
    property bool darkTheme: UtilsAdapter.getAppValue(Settings.EnableDarkTheme)

    // Jami theme colors
    function rgba256(r, g, b, a) {
        return Qt.rgba(r / 255, g / 255, b / 255, a / 100.)
    }

    // General
    property color blackColor: "#000000"
    property color redColor: "red"
    property color whiteColor: "#ffffff"
    property color darkGreyColor: "#272727"
    property color darkGreyColorOpacity: "#be272727" // 77%

    property color transparentColor: "transparent"
    property color primaryForegroundColor: darkTheme ? whiteColor : blackColor
    property color primaryBackgroundColor: darkTheme ? bgDarkMode_ : whiteColor
    property color backgroundColor: darkTheme? bgSideBarDarkMode_ : lightGrey_
    property color shadowColor: "#80000000"
    property color secondaryBackgroundColor: darkTheme ? bgDarkMode_ : whiteColor
    property color greyBorderColor: "#333"
    property color selectionBlue: darkTheme? "#0061a5" : "#109ede"

    property color hoverColor: darkTheme ? "#515151" : "#c7c7c7"
    property color pressColor: darkTheme ? "#777" : "#c0c0c0"
    property color selectedColor: darkTheme ? "#0e81c5" : "#e0e0e0"
    property color editBackgroundColor: darkTheme ? "#373737" : lightGrey_
    property color textColor: primaryForegroundColor
    property color tabbarBorderColor: darkTheme ? blackColor : "#e3e3e3"

    // Side panel
    property color presenceGreen: "#4cd964"
    property color notificationRed: "#ff3b30"
    property color notificationBlue: "#31b7ff"
    property color unPresenceOrange: "orange"
    property color placeHolderTextFontColor: "#767676"
    property color draftTextColor: "#cf5300"
    property color selectedTabColor: primaryForegroundColor
    property color filterBadgeColor: "#eed4d8"
    property color filterBadgeTextColor: "#cc0022"

    // General buttons
    property color pressedButtonColor: darkTheme ? pressColor : "#a0a0a0"
    property color hoveredButtonColor: darkTheme ? hoverColor : "#c7c7c7"
    property color normalButtonColor: darkTheme ? backgroundColor : "#e0e0e0"

    property color invertedPressedButtonColor: Qt.rgba(0, 0, 0, 0.5)
    property color invertedHoveredButtonColor: Qt.rgba(0, 0, 0, 0.6)
    property color invertedNormalButtonColor: Qt.rgba(0, 0, 0, 0.75)

    property color buttonTintedBlue: "#00aaff"
    property color buttonTintedBlueHovered: "#0e81c5"
    property color buttonTintedBluePressed: "#273261"
    property color buttonTintedGrey: darkTheme ? "#555" : "#999"
    property color buttonTintedGreyHovered: "#777"
    property color buttonTintedGreyPressed: "#777"
    property color buttonTintedGreyInactive: darkTheme ? "#777" : "#bbb"
    property color buttonTintedBlack: darkTheme ? "#fff" : "#333"
    property color buttonTintedBlackHovered: darkTheme ? "#ddd" : "#111"
    property color buttonTintedBlackPressed: darkTheme ? "#ddd" : "#000"
    property color buttonTintedRed: "red"
    property color buttonTintedRedHovered: "#c00"
    property color buttonTintedRedPressed: "#b00"
    property color acceptGreen: rgba256(11, 130, 113, 100)
    property color acceptGreenTransparency: rgba256(11, 130, 113, 56)
    property color refuseRed: rgba256(204, 0, 34, 100)
    property color refuseRedTransparent: rgba256(204, 0, 34, 56)
    property color mosaicButtonNormalColor: "#272727"
    property color whiteColorTransparent: rgba256(255, 255, 255, 50)
    property color raiseHandColor: rgba256(0, 184, 255, 77)

    property color closeButtonLighterBlack: "#4c4c4c"

    // Jami switch
    property color switchBackgroundBorderColor: "#cccccc"
    property color switchBackgroundCheckedColor: primaryBackgroundColor
    property color switchIndicatorBorderColor: "#999999"

    // Call buttons
    property color acceptButtonGreen: "#4caf50"
    property color acceptButtonHoverGreen: "#5db761"
    property color acceptButtonPressedGreen: "#449d48"

    property color declineButtonRed: "#f44336"
    property color declineButtonHoverRed: "#f5554a"
    property color declineButtonPressedRed: "#db3c30"

    property color sipInputButtonBackgroundColor: "#336699"
    property color sipInputButtonHoverColor: "#4477aa"
    property color sipInputButtonPressColor: "#5588bb"

    property string buttonConference: "transparent"
    property string buttonConferenceHovered:"#110000"
    property string buttonConferencePressed: "#110000"

    // Wizard / account manager
    property color accountCreationOtherStepColor: "grey"
    property color accountCreationCurrentStepColor: "#28b1ed"
    property color wizardBlueButtons: "#28b1ed"
    property color wizardGreenColor: "#aed581"
    property color requiredFieldColor: "#ff1f62"
    property color requiredFieldBackgroundColor: "#fee4e9"

    // Misc
    property color recordIconColor: "#dc2719"
    property color successLabelColor: "#2b5084"
    property color rubberBandSelectionBlue: "steelblue"
    property color screenSelectionBorderGreen: "green"
    property color separationLine: darkTheme ? selectedColor : backgroundColor

    // Plugin Preferences View
    property color comboBoxBackgroundColor: darkTheme ? editBackgroundColor : selectedColor

    // ParticipantCallInStatusView
    property color participantCallInStatusTextColor: whiteColor

    // InvitationView
    property color blockOrange: rgba256(232, 92, 36, 100)
    property color blockOrangeTransparency: rgba256(232, 92, 36, 56)

    // Chatview
    property color jamiLightBlue: darkTheme ? "#003b4e" : Qt.rgba(59, 193, 211, 0.3)
    property color jamiDarkBlue: darkTheme ? "#28b1ed" : "#003b4e"
    property color chatviewTextColor: darkTheme ? "#f0f0f0" : "#353637"
    property color timestampColor: darkTheme ? "#bbb" : "#777"
    property color messageOutBgColor: darkTheme ? "#28b1ed" : "#cfd8dc"
    property color messageOutTxtColor: chatviewTextColor
    property color messageInBgColor: darkTheme? "#616161" : "#cfebf5"
    property color messageInTxtColor: chatviewTextColor
    property color fileOutTimestampColor: darkTheme ? "#eee" : "#555"
    property color fileInTimestampColor: darkTheme ? "#999" : "#555"
    property color chatviewBgColor: darkTheme ? bgDarkMode_ : whiteColor
    property color bgInvitationRectColor: darkTheme ? "#222222" : whiteColor
    property color placeholderTextColor: darkTheme ? "#2b2b2b" : "#d3d3d3"
    property color inviteHoverColor: darkTheme ? blackColor : whiteColor
    property color chatviewButtonColor: darkTheme ? whiteColor : blackColor
    property color bgTextInput: darkTheme ? "#060608" : whiteColor
    property color previewTextContainerColor: darkTheme ? "#959595" : "#ececec"
    property color previewTitleColor: darkTheme ? whiteColor : blackColor
    property color previewSubtitleColor: darkTheme ? whiteColor : blackColor
    property color previewImageBackgroundColor: whiteColor
    property color previewCardContainerColor : darkTheme ? blackColor : whiteColor
    property color previewUrlColor : darkTheme ? "#eeeeee" : "#333"
    property color messageWebViewFooterButtonImageColor: darkTheme ? "#838383" : "#656565"

    // Files To Send Container
    property color removeFileButtonColor: Qt.rgba(96, 95, 97, 0.5)

    // TypingDots
    property color typingDotsNormalColor: darkTheme ? "#686b72" : "lightgrey"
    property color typingDotsEnlargeColor: darkTheme ? "white" : Qt.darker("lightgrey", 3.0)

    // Font.
    property color faddedFontColor: darkTheme? "#c0c0c0" : "#a0a0a0"
    property color faddedLastInteractionFontColor: darkTheme ? "#c0c0c0" : "#505050"

    property color darkGrey: rgba256(63, 63, 63, 100)
    property color blueLogo_: darkTheme ? whiteColor : rgba256(0, 7, 71, 100)
    property color lightGrey_: rgba256(242, 242, 242, 100)
    property color mediumGrey: rgba256(218, 219, 220, 100)
    property color grey_: rgba256(160, 160, 160, 100)
    property color red_: rgba256(251, 72, 71, 100)
    property color urgentOrange_: rgba256(255, 165, 0, 100)
    property color green_: rgba256(127, 255, 0, 100)
    property color presenceGreen_: rgba256(76, 217, 100, 100)
    property color bgSideBarDarkMode_: rgba256(24, 24, 24, 100)
    property color bgDarkMode_: rgba256(32, 32, 32, 100)

    property int shortFadeDuration: 150
    property int recordBlinkDuration: 500
    property int overlayFadeDelay: 4000
    property int overlayFadeDuration: 250
    property int smartListTransitionDuration: 120

    // Sizes
    property real splitViewHandlePreferredWidth: 4
    property real indicatorFontSize: 6
    property real tinyFontSize: 7
    property real textFontSize: 9
    property real settingsFontSize: 9
    property real buttonFontSize: 9
    property real participantFontSize: 10
    property real menuFontSize: 12
    property real headerFontSize: 13
    property real titleFontSize: 16
    property real primaryRadius: 4
    property real smartlistItemFontSize: 10.5
    property real smartlistItemInfoFontSize: 9
    property real filterItemFontSize: smartlistItemFontSize
    property real filterBadgeFontSize: 8.25
    property real accountListItemHeight: 64
    property real accountListAvatarSize: 40
    property real smartListItemHeight: 64
    property real smartListAvatarSize: 52
    property real avatarSizeInCall: 130
    property real aboutButtonPreferredWidth: 150
    property real callButtonPreferredSize: 50
    property real contextMenuItemTextPreferredWidth: 152
    property real contextMenuItemTextMaxWidth: 182
    property int participantCallInStatusViewWidth: 175
    property int participantCallInStatusViewHeight: 300
    property int participantCallInStatusDelegateHeight: 85
    property int participantCallInStatusDelegateRadius: 5
    property real participantCallInStatusOpacity: 0.77
    property int participantCallInAvatarSize: 60
    property int participantCallInNameFontSize: 11
    property int participantCallInStatusFontSize: 7
    property int participantCallInStatusTextWidthLimit: 80
    property int participantCallInStatusTextWidth: 40
    property int mosaicButtonRadius: 5
    property int mosaicButtonPreferredMargin: 5
    property real mosaicButtonOpacity: 0.77
    property int mosaicButtonTextPreferredWidth: 40
    property int mosaicButtonTextPreferredHeight: 16
    property int mosaicButtonTextPointSize: 8
    property int mosaicButtonPreferredWidth: 70
    property int mosaicButtonMaxWidth: 100
    property real avatarPresenceRatio: 0.26
    property int avatarReadReceiptSize: 18

    property int menuItemsPreferredWidth: 220
    property int menuItemsPreferredHeight: 48
    property int menuItemsCommonBorderWidth: 1
    property int menuBorderPreferredHeight: 8

    property real maximumWidthSettingsView: 600
    property real settingsHeaderpreferredHeight: 64
    property real preferredFieldWidth: 256
    property real preferredFieldHeight: 32
    property real preferredMarginSize: 16
    property real preferredDialogWidth: 400
    property real preferredDialogHeight: 300
    property real minimumPreviewWidth: 120
    property real pluginHandlersPopupViewHeight: 200
    property real pluginHandlersPopupViewDelegateHeight: 50

    property real lineEditContextMenuItemsHeight: 15
    property real lineEditContextMenuItemsWidth: 100
    property real lineEditContextMenuSeparatorsHeight: 2

    // Jami switch
    property real switchIndicatorRadius: 13
    property real switchPreferredHeight: 18
    property real switchPreferredWidth: 48
    property real switchIndicatorPreferredHeight: 26
    property real switchIndicatorPreferredWidth: 26

    // Modal Popup
    property real modalPopupRadius: 4
    property real modalPopupDropShadowSamples: 16

    // MessageWebView
    property real chatViewHairLineSize: 1
    property real chatViewMaximumWidth: 900
    property real chatViewHeaderPreferredHeight: 64
    property real chatViewFooterPreferredHeight: 50
    property real chatViewFooterMaximumHeight: 280
    property real chatViewFooterRowSpacing: 1
    property real chatViewFooterButtonSize: 36
    property real chatViewFooterButtonIconSize: 48
    property real chatViewFooterButtonRadius: 5
    property real chatViewFooterFileContainerPreferredHeight: 150
    property real chatViewFooterTextAreaMaximumHeight: 130
    property real chatViewScrollToBottomButtonBottomMargin: 8

    // TypingDots
    property real typingDotsAnimationInterval: 500
    property real typingDotsRadius: 30
    property real typingDotsSize: 8

    // MessageWebView File Transfer Container
    property real filesToSendContainerSpacing: 5
    property real filesToSendContainerPadding: 10
    property real filesToSendDelegateWidth: 100
    property real filesToSendDelegateHeight: 100
    property real filesToSendDelegateRadius: 7
    property real filesToSendDelegateButtonSize: 16
    property real filesToSendDelegateFontPointSize: textFontSize + 2

    // SBSMessageBase
    property int sbsMessageBasePreferredPadding: 12

    // MessageBar
    property int messageBarMarginSize: 10

    // InvitationView
    property real invitationViewAvatarSize: 112
    property real invitationViewButtonRadius: 25
    property real invitationViewButtonSize: 48
    property real invitationViewButtonIconSize: 24
    property real invitationViewButtonsSpacing: 30

    // WizardView
    property real wizardViewPageLayoutSpacing: 12
    property real wizardViewPageBackButtonMargins: 20
    property real wizardViewPageBackButtonSize: 35

    // WizardView Welcome Page
    property real welcomeLabelPointSize: 30
    property real welcomeLogoWidth: 330
    property real welcomeLogoHeight: 110
    property real wizardButtonWidth: 400

    // MaterialLineEdit
    property real materialLineEditPointSize: 10
    property real materialLineEditPadding: 16

    // UsernameLineEdit
    property real usernameLineEditPointSize: 9
    property real usernameLineEditlookupInterval: 200

    // Main application spec
    property real mainViewMinWidth: 332
    property real mainViewMinHeight: 500

    property real wizardViewMinWidth: 500
    property real wizardViewMinHeight: 600

    property real mainViewPreferredWidth: 725
    property real mainViewPreferredHeight: 600

    function setTheme(dark) {
        darkTheme = dark
    }
}
