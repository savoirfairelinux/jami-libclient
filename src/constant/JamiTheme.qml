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
    // Color strings.
    property string hoverColor: "#c7c7c7"
    property string pressColor: "#c0c0c0"
    property string releaseColor: "#e0e0e0"
    property string tabbarBorderColor: "#e3e3e3"
    property string transparentColor: "transparent"
    property string presenceGreen: "#4cd964"
    property string notificationRed: "#ff3b30"
    property string unPresenceOrange: "orange"
    property string backgroundColor: lightGrey_
    property string backgroundDarkColor: lightGreyTab_

    property string screenSelectionBorderGreen: "green"

    property string acceptButtonGreen: "#4caf50"
    property string acceptButtonHoverGreen: "#5db761"
    property string acceptButtonPressedGreen: "#449d48"

    property string declineButtonRed: "#f44336"
    property string declineButtonHoverRed: "#f5554a"
    property string declineButtonPressedRed: "#db3c30"

    property string hangUpButtonTintedRed: "#ff0000"
    property string buttonTintedBlue: "#00aaff"
    property string buttonTintedBlueHovered: "#0e81c5"
    property string buttonTintedBluePressed: "#273261"
    property string buttonTintedGrey: "#999"
    property string buttonTintedGreyHovered: "#777"
    property string buttonTintedGreyPressed: "#777"
    property string buttonTintedGreyInactive: "#bbb"
    property string buttonTintedBlack: "#333"
    property string buttonTintedBlackHovered: "#111"
    property string buttonTintedBlackPressed: "#000"
    property string buttonTintedRed: "red"
    property string buttonTintedRedHovered: "#c00"
    property string buttonTintedRedPressed: "#b00"

    property string selectionBlue: "#109ede"
    property string selectionGreen: "#21be2b"
    property string rubberBandSelectionBlue: "steelblue"

    property string closeButtonLighterBlack: "#4c4c4c"

    property string contactSearchBarPlaceHolderTextFontColor: "#767676"
    property string contactSearchBarPlaceHolderGreyBackground: "#dddddd"

    property string draftRed: "#cf5300"

    property string sipInputButtonBackgroundColor: "#336699"
    property string sipInputButtonHoverColor: "#4477aa"
    property string sipInputButtonPressColor: "#5588bb"

    property string accountCreationOtherStepColor: "grey"
    property string accountCreationCurrentStepColor: "#28b1ed"

    // Font.
    property string faddedFontColor: "#c0c0c0"
    property string faddedLastInteractionFontColor: "#505050"

    property int splitViewHandlePreferredWidth: 4
    property int textFontSize: 9
    property int settingsFontSize: 9
    property int buttonFontSize: 9
    property int headerFontSize: 13
    property int titleFontSize: 16
    property int menuFontSize: 12

    property int maximumWidthSettingsView: 800
    property int preferredFieldWidth: 236
    property int preferredFieldHeight: 32
    property int preferredMarginSize: 16

    // Place holder text.
    property string contactSearchBarPlaceHolderConversationText: qsTr("Find or start a conversation")
    property string contactSearchBarPlaceHolderInivitionText: qsTr("Search your invitations")

    // Jami theme colors
    function rgb256(r, g, b) {
        return Qt.rgba(r / 256, g / 256, b / 256, 1)
    }

    property color blue_: "#109ede"
    property color wizardBlueButtons: "#28b1ed"
    property color blueLogo_: rgb256(0, 7, 71)
    property color lightBlue_: "#c1ebf0"
    property color lightGrey_: rgb256(242, 242, 242)
    property color lightGreyTab_: rgb256(220, 220, 220)
    property color imGrey_: "#dedee0"
    property color imBlue_: "#cfebf5"
    property color lightBlack_: rgb256(63, 63, 63)
    property color grey_: rgb256(160, 160, 160)
    property color red_: rgb256(251, 72, 71)
    property color lightRed_: rgb256(252, 91, 90)
    property color darkRed_: rgb256(219, 55, 54)
    property color notificationRed_: rgb256(255, 59, 48)
    property color urgentOrange_: rgb256(255, 165, 0)
    property color green_: rgb256(127, 255, 0)
    property color presenceGreen_: rgb256(76, 217, 100)
    property color smartlistSelection_: rgb256(240, 240, 240)
    property color smartlistHighlight_: rgb256(245, 245, 245)
}
