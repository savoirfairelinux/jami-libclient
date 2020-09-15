
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
import QtGraphicalEffects 1.12
import net.jami.Models 1.0

Rectangle {
    id: contactSearchBarRect

    signal contactSearchBarTextChanged(string text)

    // Hack - there is no real way now to make TextField lose its focus,
    // unless transfer it to other component.
    function clearFocus() {
        fakeFocus.forceActiveFocus()
    }

    function clearText() {
        contactSearchBar.clear()
        fakeFocus.forceActiveFocus()
    }

    radius: height/2
    color: "white"

    FocusScope {
        id: fakeFocus
    }

    Image {
        id: searchIconImage

        anchors.verticalCenter: contactSearchBarRect.verticalCenter
        anchors.left: contactSearchBarRect.left
        anchors.leftMargin: 8

        width: 20
        height: 20

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

        anchors.verticalCenter: contactSearchBarRect.verticalCenter
        anchors.left: searchIconImage.right

        width: contactSearchBarRect.width - searchIconImage.width - 10
        height: contactSearchBarRect.height - 5

        font.pointSize: JamiTheme.textFontSize
        selectByMouse: true
        selectionColor: JamiTheme.contactSearchBarPlaceHolderTextFontColor

        placeholderText: JamiStrings.contactSearchConversation
        placeholderTextColor: JamiTheme.contactSearchBarPlaceHolderTextFontColor

        background: Rectangle {
            id: searchBarBackground

            color: "transparent"
        }

        onTextChanged: {
            contactSearchBarRect.contactSearchBarTextChanged(
                        contactSearchBar.text)
        }
    }

    Shortcut {
        sequence: "Ctrl+F"
        context: Qt.ApplicationShortcut
        onActivated: contactSearchBar.forceActiveFocus()
    }
}
