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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import net.jami.Adapters 1.1
import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

// SipInputPanel is a key pad that is designed to be
// used in sip calls.
Popup {
    id: sipInputPanelPopUp

    // Space between sipInputPanelRect and grid layout
    property int sipPanelPadding: 20

    contentWidth: sipInputPanelRectGridLayout.implicitWidth + 20
    contentHeight: sipInputPanelRectGridLayout.implicitHeight + 20

    padding: 0

    modal: true

    contentItem: Rectangle {
        id: sipInputPanelRect

        radius: 10

        GridLayout {
            id: sipInputPanelRectGridLayout

            anchors.centerIn: parent

            columns: 4

            Repeater {
                id: sipInputPanelRectGridLayoutRepeater
                model: ["1", "2", "3", "A", "4", "5", "6", "B", "7",
                        "8", "9", "C", "*", "0", "#", "D"]

                PushButton {
                    id: sipInputPanelButton

                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30

                    preferredMargin: 8
                    buttonText: modelData
                    buttonTextColor: "white"
                    checkable: false

                    pressedColor: JamiTheme.sipInputButtonPressColor
                    hoveredColor: JamiTheme.sipInputButtonHoverColor
                    normalColor: JamiTheme.sipInputButtonBackgroundColor

                    toolTipText: modelData

                    onClicked: {
                        CallAdapter.sipInputPanelPlayDTMF(modelData)
                    }
                }
            }
        }
    }

    background: Rectangle {
        color: "transparent"
    }
}
