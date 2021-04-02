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
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

Rectangle {
    id: root

    property int itemMargin: 8

    signal contactSearchBarTextChanged(string text)
    signal returnPressedWhileSearching

    function clearText() {
        contactSearchBar.clear()
        fakeFocus.forceActiveFocus()
    }

    radius: JamiTheme.lineEditRadius
    color: JamiTheme.secondaryBackgroundColor

    FocusScope {
        id: fakeFocus
    }

    Image {
        id: searchIconImage

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: itemMargin

        width: 16
        height: 16

        fillMode: Image.PreserveAspectFit
        mipmap: true
        source: "qrc:/images/icons/ic_baseline-search-24px.svg"
    }

    ColorOverlay {
        anchors.fill: searchIconImage
        source: searchIconImage
        color: JamiTheme.contactSearchBarPlaceHolderTextFontColor
    }

    TextField {
        id: contactSearchBar

        anchors.verticalCenter: root.verticalCenter
        anchors.left: searchIconImage.right

        width: root.width - searchIconImage.width - clearTextButton.width - itemMargin * 2
        height: root.height - 5

        color: JamiTheme.textColor

        font.pointSize: JamiTheme.textFontSize
        font.kerning: true

        selectByMouse: true
        selectionColor: JamiTheme.contactSearchBarPlaceHolderTextFontColor

        placeholderText: JamiStrings.contactSearchConversation
        placeholderTextColor: JamiTheme.contactSearchBarPlaceHolderTextFontColor

        background: Rectangle {
            id: searchBarBackground

            color: "transparent"
        }

        onTextChanged: root.contactSearchBarTextChanged(contactSearchBar.text)
    }

    PushButton {
        id: clearTextButton

        anchors.verticalCenter: root.verticalCenter
        anchors.left: contactSearchBar.right

        preferredSize: 16

        visible: contactSearchBar.text.length

        normalColor: root.color
        imageColor: JamiTheme.primaryForegroundColor

        source: "qrc:/images/icons/ic_clear_24px.svg"
        toolTipText: JamiStrings.clearText

        onClicked: contactSearchBar.clear()
    }

    Shortcut {
        sequence: "Ctrl+F"
        context: Qt.ApplicationShortcut
        onActivated: contactSearchBar.forceActiveFocus()
    }

    Shortcut {
        sequence: "Return"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (contactSearchBar.text !== "") {
                returnPressedWhileSearching()
            }
        }
    }
}
