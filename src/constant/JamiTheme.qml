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

import QtQuick 2.14

import net.jami.Adapters 1.0
import net.jami.Enums 1.0

Item {
    property bool darkTheme: SettingsAdapter.getAppValue(Settings.EnableDarkTheme)

    // General
    property color blackColor: "#000000"
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

    property color closeButtonLighterBlack: "#4c4c4c"

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

    // Chatview
    property color jamiLightBlue: darkTheme ? "#003b4e" : Qt.rgba(59, 193, 211, 0.3)
    property color jamiDarkBlue: darkTheme ? "#28b1ed" : "#003b4e"
    property color chatviewTextColor: textColor
    property color timestampColor: darkTheme ? "#bbb" : "#333"
    property color messageOutBgColor: darkTheme ? "#28b1ed" : "#cfd8dc"
    property color messageOutTxtColor: textColor
    property color messageInBgColor: darkTheme? "#616161" : "#cfebf5"
    property color messageInTxtColor: textColor
    property color fileOutTimestampColor: darkTheme ? "#eee" : "#555"
    property color fileInTimestampColor: darkTheme ? "#999" : "#555"
    property color chatviewBgColor: darkTheme ? bgDarkMode_ : whiteColor
    property color bgInvitationRectColor: darkTheme ? "#222222" : whiteColor
    property color placeholderTextColor: darkTheme ? "#2b2b2b" : "#d3d3d3"
    property color inviteHoverColor: darkTheme ? blackColor : whiteColor
    property color chatviewButtonColor: darkTheme ? whiteColor : blackColor
    property color bgTextInput: darkTheme ? "#060608" : whiteColor

    // Font.
    property color faddedFontColor: darkTheme? "#c0c0c0" : "#a0a0a0"
    property color faddedLastInteractionFontColor: darkTheme ? "#c0c0c0" : "#505050"

    // Jami theme colors
    function rgb256(r, g, b) {
        return Qt.rgba(r / 255, g / 255, b / 255, 1.0)
    }

    property color darkGrey: rgb256(63, 63, 63)
    property color blueLogo_: darkTheme ? whiteColor : rgb256(0, 7, 71)
    property color lightGrey_: rgb256(242, 242, 242)
    property color mediumGrey: rgb256(218, 219, 220)
    property color grey_: rgb256(160, 160, 160)
    property color red_: rgb256(251, 72, 71)
    property color urgentOrange_: rgb256(255, 165, 0)
    property color green_: rgb256(127, 255, 0)
    property color presenceGreen_: rgb256(76, 217, 100)
    property color bgSideBarDarkMode_: rgb256(24, 24, 24)
    property color bgDarkMode_: rgb256(32, 32, 32)

    property int fadeDuration: 150

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
    property real smartListTransitionDuration: 120

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

    // main application spec
    property real mainViewMinWidth: 300
    property real mainViewMinHeight: 400

    property real wizardViewMinWidth: 500
    property real wizardViewMinHeight: 600

    property real mainViewPreferredWidth: 725
    property real mainViewPreferredHeight: 600

    function setTheme(dark) {
        darkTheme = dark
    }
}
