/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.3

MessageDialog {
    id: messageBox

    visible: false
    modality:  Qt.NonModal
    width: 300
    height: 200

    function openWithParameters(titleToDisplay, infoToDisplay, infoIconMode = StandardIcon.Information, buttons = StandardButton.Ok){
        title = titleToDisplay
        text = infoToDisplay
        icon = infoIconMode
        standardButtons = buttons
        messageBox.open()
    }
}
