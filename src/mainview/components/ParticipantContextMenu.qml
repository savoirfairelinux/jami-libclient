
/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtGraphicalEffects 1.12
import net.jami.Models 1.0

import "../../commoncomponents"

import "../js/videodevicecontextmenuitemcreation.js" as VideoDeviceContextMenuItemCreation
import "../js/selectscreenwindowcreation.js" as SelectScreenWindowCreation

Menu {
    id: root

    property int generalMenuSeparatorCount: 0
    property int commonBorderWidth: 1
    font.pointSize: JamiTheme.textFontSize + 3
    property var uri: ""
    property var maximized: true
    property var active: true

    function showHangup(show) {
        if (show) {
            hangupItem.visible = true
            hangupItem.height = hangupItem.preferredHeight
        } else {
            hangupItem.visible = false
            hangupItem.height = 0
        }
    }

    function showMaximize(show) {
        if (show) {
            maximizeItem.visible = true
            maximizeItem.height = hangupItem.preferredHeight
        } else {
            maximizeItem.visible = false
            maximizeItem.height = 0
        }
    }

    function showMinimize(show) {
        if (show) {
            minimizeItem.visible = true
            minimizeItem.height = hangupItem.preferredHeight
        } else {
            minimizeItem.visible = false
            minimizeItem.height = 0
        }
    }

    function setHeight(visibleItems) {
        root.height = hangupItem.preferredHeight * visibleItems;
    }

    /*
     * All GeneralMenuItems should remain the same width / height.
     */
    GeneralMenuItem {
        id: hangupItem

        itemName: qsTr("Hangup")
        iconSource: "qrc:/images/icons/ic_call_end_white_24px.svg"
        icon.color: "black"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            CallAdapter.hangupCall(uri)
            root.close()
        }
    }
    GeneralMenuItem {
        id: maximizeItem

        itemName: qsTr("Maximize participant")
        iconSource: "qrc:/images/icons/open_in_full-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth
        visible: !maximized

        onClicked: {
            CallAdapter.maximizeParticipant(uri, active)
            root.close()
        }
    }
    GeneralMenuItem {
        id: minimizeItem

        itemName: qsTr("Minimize participant")
        iconSource: "qrc:/images/icons/close_fullscreen-24px.svg"
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth
        visible: maximized

        onClicked: {
            CallAdapter.minimizeParticipant()
            root.close()
        }
    }

    background: Rectangle {
        implicitWidth: hangupItem.preferredWidth
        implicitHeight: hangupItem.preferredHeight * 3

        border.width: commonBorderWidth
        border.color: JamiTheme.tabbarBorderColor
    }
}

