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

import net.jami.Models 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    signal contactSearchBarTextChanged(string text)
    signal returnPressedWhileSearching

    property alias textContent: contactSearchBar.text
    property alias placeHolderText: contactSearchBar.placeholderText

    function clearText() {
        contactSearchBar.clear()
        fakeFocus.forceActiveFocus()
    }

    radius: JamiTheme.primaryRadius
    color: JamiTheme.secondaryBackgroundColor

    FocusScope {
        id: fakeFocus
    }

    LineEditContextMenu {
        id: lineEditContextMenu

        lineEditObj: contactSearchBar
    }

    ResponsiveImage {
        id: searchIconImage

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 10

        width: 20
        height: 20

        source: JamiResources.ic_baseline_search_24dp_svg
        color: JamiTheme.primaryForegroundColor
    }

    TextField {
        id: contactSearchBar

        anchors.verticalCenter: root.verticalCenter
        anchors.left: searchIconImage.right
        anchors.right: contactSearchBar.text.length ?
                           clearTextButton.left :
                           root.right

        height: root.height - 5

        color: JamiTheme.textColor

        font.pointSize: JamiTheme.textFontSize
        font.kerning: true

        selectByMouse: true
        selectionColor: JamiTheme.placeHolderTextFontColor

        placeholderText: JamiStrings.contactSearchConversation
        placeholderTextColor: JamiTheme.placeHolderTextFontColor

        background: Rectangle {
            id: searchBarBackground

            color: "transparent"
        }

        onTextChanged: root.contactSearchBarTextChanged(contactSearchBar.text)
        onReleased: function (event) {
            if (event.button === Qt.RightButton)
                lineEditContextMenu.openMenuAt(event)
        }
    }

    PushButton {
        id: clearTextButton

        anchors.verticalCenter: root.verticalCenter
        anchors.right: root.right
        anchors.rightMargin: 10

        preferredSize: 21
        radius: JamiTheme.primaryRadius

        visible: contactSearchBar.text.length
        opacity: visible ? 1 : 0

        normalColor: root.color
        imageColor: JamiTheme.primaryForegroundColor

        source: JamiResources.ic_clear_24dp_svg
        toolTipText: JamiStrings.clearText

        onClicked: contactSearchBar.clear()

        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.OutCubic }
        }
    }

    Shortcut {
        sequence: "Ctrl+F"
        context: Qt.ApplicationShortcut
        onActivated: contactSearchBar.forceActiveFocus()
    }

    Keys.onPressed: function (keyEvent) {
        if (keyEvent.key === Qt.Key_Enter ||
                keyEvent.key === Qt.Key_Return) {
            if (contactSearchBar.text !== "") {
                returnPressedWhileSearching()
                keyEvent.accepted = true
            }
        }
    }
}
