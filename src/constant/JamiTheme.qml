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

Item {
    // General
    property color blackColor: "#000000"
    property color whiteColor: "#ffffff"
    property color transparentColor: "transparent"
    property color primaryForegroundColor: blackColor
    property color primaryBackgroundColor: whiteColor
    property color backgroundColor: lightGrey_
    property color shadowColor: "#80000000"
    property color secondaryBackgroundColor: "white"
    property color greyBorderColor: "#333"
    property color selectionBlue: "#109ede"

    property color hoverColor: "#c7c7c7"
    property color pressColor: "#c0c0c0"
    property color selectedColor: "#e0e0e0"
    property color editBackgroundColor: lightGrey_
    property color textColor: primaryForegroundColor
    property color tabbarBorderColor: "#e3e3e3"

    // Side panel
    property color presenceGreen: "#4cd964"
    property color notificationRed: "#ff3b30"
    property color unPresenceOrange: "orange"
    property color contactSearchBarPlaceHolderTextFontColor: "#767676"
    property color draftRed: "#cf5300"

    // General buttons
    property color pressedButtonColor: "#a0a0a0"
    property color hoveredButtonColor: "#c7c7c7"
    property color normalButtonColor: "#e0e0e0"

    property color invertedPressedButtonColor: Qt.rgba(0, 0, 0, 0.5)
    property color invertedHoveredButtonColor: Qt.rgba(0, 0, 0, 0.6)
    property color invertedNormalButtonColor: Qt.rgba(0, 0, 0, 0.75)

    property color buttonTintedBlue: "#00aaff"
    property color buttonTintedBlueHovered: "#0e81c5"
    property color buttonTintedBluePressed: "#273261"
    property color buttonTintedGrey: "#999"
    property color buttonTintedGreyHovered: "#777"
    property color buttonTintedGreyPressed: "#777"
    property color buttonTintedGreyInactive: "#bbb"
    property color buttonTintedBlack: "#333"
    property color buttonTintedBlackHovered: "#111"
    property color buttonTintedBlackPressed: "#000"
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

    // Font.
    property color faddedFontColor: "#a0a0a0"
    property color faddedLastInteractionFontColor: "#505050"

    // Jami theme colors
    function rgb256(r, g, b) {
        return Qt.rgba(r / 255, g / 255, b / 255, 1.0)
    }

    property color darkGrey: rgb256(63, 63, 63)
    property color blueLogo_: rgb256(0, 7, 71)
    property color lightGrey_: rgb256(242, 242, 242)
    property color grey_: rgb256(160, 160, 160)
    property color red_: rgb256(251, 72, 71)
    property color urgentOrange_: rgb256(255, 165, 0)
    property color green_: rgb256(127, 255, 0)
    property color presenceGreen_: rgb256(76, 217, 100)

    property int fadeDuration: 150

    // Sizes
    property int splitViewHandlePreferredWidth: 4
    property int tinyFontSize: 7
    property int textFontSize: 9
    property int settingsFontSize: 9
    property int buttonFontSize: 9
    property int menuFontSize: 12
    property int headerFontSize: 13
    property int titleFontSize: 16

    property int maximumWidthSettingsView: 600
    property int settingsHeaderpreferredHeight: 64
    property int preferredFieldWidth: 256
    property int preferredFieldHeight: 32
    property int preferredMarginSize: 16
    property int preferredDialogWidth: 400
    property int preferredDialogHeight: 300
    property int minimumPreviewWidth: 120
}
