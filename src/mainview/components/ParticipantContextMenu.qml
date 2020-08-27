/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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
import QtGraphicalEffects 1.12
import net.jami.Models 1.0

import "../../commoncomponents"

import "../../commoncomponents/js/contextmenugenerator.js" as ContextMenuGenerator

Item {
    id: root

    property var uri: ""
    property var maximized: true
    property var active: true
    property var showHangup: false
    property var showMaximize: false
    property var showMinimize: false

    function openMenu(){
        if (showHangup)
            ContextMenuGenerator.addMenuItem(qsTr("Hang up"),
                                             "qrc:/images/icons/ic_call_end_white_24px.svg",
                                             function (){
                                                 CallAdapter.hangupCall(uri)
                                             })

        if (showMaximize)
            ContextMenuGenerator.addMenuItem(qsTr("Maximize participant"),
                                             "qrc:/images/icons/open_in_full-24px.svg",
                                             function (){
                                                  CallAdapter.maximizeParticipant(uri, active)
                                             })
        if (showMinimize)
            ContextMenuGenerator.addMenuItem(qsTr("Minimize participant"),
                                             "qrc:/images/icons/close_fullscreen-24px.svg",
                                             function (){
                                                  CallAdapter.minimizeParticipant()
                                             })

        root.height = ContextMenuGenerator.getMenu().height
        root.width = ContextMenuGenerator.getMenu().width
        ContextMenuGenerator.getMenu().open()
    }

    Component.onCompleted: {
        ContextMenuGenerator.createBaseContextMenuObjects(root)
    }
}

