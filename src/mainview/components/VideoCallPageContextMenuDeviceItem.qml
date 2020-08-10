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
import net.jami.Models 1.0

import "../../commoncomponents"

/*
 * Menu item wrapper for video device checkable item.
 */
GeneralMenuItem {
    id: videoCallPageContextMenuDeviceItem

    property int contextMenuPreferredWidth: 250

    TextMetrics {
        id: textMetrics
        elide: Text.ElideMiddle
        elideWidth: contextMenuPreferredWidth
                    - videoCallPageContextMenuDeviceItem.implicitIndicatorWidth
        text: videoCallPageContextMenuDeviceItem.itemName
    }

    itemName: textMetrics.elidedText.length !== 0 ?
        textMetrics.elidedText :
        qsTr("No video device")

    indicator: null

    iconSource: videoCallPageContextMenuDeviceItem.checked ?
        "qrc:/images/icons/check_box-24px.svg" :
        "qrc:/images/icons/check_box_outline_blank-24px.svg"

    onClicked: {
        var deviceName = videoCallPageContextMenuDeviceItem.itemName
        AvAdapter.onVideoContextMenuDeviceItemClicked(deviceName)
    }
}