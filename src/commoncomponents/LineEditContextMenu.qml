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
import QtGraphicalEffects 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "js/contextmenugenerator.js" as ContextMenuGenerator

Item {
    id: root

    function openMenu(lineEditObj, mouseEvent) {
        ContextMenuGenerator.initMenu(Qt.size(150, 25), 2)

        if (lineEditObj.selectedText.length) {
            ContextMenuGenerator.addMenuItem(qsTr("Copy"),
                                             "",
                                             function (){
                                                 lineEditObj.copy()
                                             })

            ContextMenuGenerator.addMenuItem(qsTr("Cut"),
                                             "",
                                             function (){
                                                 lineEditObj.cut()
                                             })
        }

        ContextMenuGenerator.addMenuItem(qsTr("Paste"),
                                         "",
                                         function (){
                                             lineEditObj.paste()
                                         })

        root.height = ContextMenuGenerator.getMenu().height
        root.width = ContextMenuGenerator.getMenu().width
        ContextMenuGenerator.getMenu().x = mouseEvent.x
        ContextMenuGenerator.getMenu().y = mouseEvent.y

        // lineEdit (TextEdit) selection will be lost when menu is opened
        var selectionStartTemp = lineEditObj.selectionStart
        var selectionEndTemp = lineEditObj.selectionEnd

        ContextMenuGenerator.getMenu().open()

        lineEditObj.select(selectionStartTemp, selectionEndTemp)
    }

    Component.onCompleted: {
        ContextMenuGenerator.createBaseContextMenuObjects(root)
    }
}
