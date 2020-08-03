
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
 * Take advantage of child can access parent's item (ex: contextMenu, commonBorderWidth).
 */
GeneralMenuItem {
    id: videoCallPageContextMenuDeviceItem

    property int contextMenuPreferredWidth: 250

    itemName: qsTr("No video device")
    leftBorderWidth: commonBorderWidth
    rightBorderWidth: commonBorderWidth

    TextMetrics {
        id: textMetrics
        font: deviceNameText.font
        elide: Text.ElideMiddle
        elideWidth: contextMenuPreferredWidth
                    - videoCallPageContextMenuDeviceItem.implicitIndicatorWidth
        text: videoCallPageContextMenuDeviceItem.itemName
    }

    contentItem: Text {
        id: deviceNameText

        leftPadding: 30
        rightPadding: videoCallPageContextMenuDeviceItem.arrow.width

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        font.pointSize: JamiTheme.textFontSize - 3
        text: textMetrics.elidedText
    }

    indicator: Item {
        id: selectItem

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        implicitWidth: 32
        implicitHeight: 32

        Rectangle {
            id: selectRect

            width: selectItem.width / 2
            height: selectItem.height / 2
            anchors.centerIn: parent
            visible: videoCallPageContextMenuDeviceItem.checkable
            border.color: JamiTheme.selectionGreen
            radius: 3
            Rectangle {
                width: selectRect.width / 2
                height: selectRect.height / 2
                anchors.centerIn: parent
                visible: videoCallPageContextMenuDeviceItem.checked
                color: JamiTheme.selectionGreen
                radius: 2
            }
        }
    }

    onClicked: {
        var deviceName = videoCallPageContextMenuDeviceItem.itemName
        contextMenu.close()
        AvAdapter.onVideoContextMenuDeviceItemClicked(deviceName)
    }
}
