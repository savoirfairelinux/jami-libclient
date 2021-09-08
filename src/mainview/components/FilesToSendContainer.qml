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

    property alias filesToSendListModel: repeater.model
    property alias filesToSendCount: repeater.count

    color: JamiTheme.messageOutBgColor

    JamiFlickable {
        id: filesToSendContainerScrollView

        anchors.fill: root

        contentHeight: root.height
        contentWidth: filesToSendContainerRow.width

        horizontalHandleColor:
            filesToSendContainerScrollView.ScrollBar.horizontal.pressed ?
                JamiTheme.darkGreyColor : JamiTheme.whiteColor
        ScrollBar.vertical.policy: ScrollBar.AlwaysOff

        Row {
            id: filesToSendContainerRow

            anchors.centerIn: parent

            spacing: JamiTheme.filesToSendContainerSpacing
            padding: JamiTheme.filesToSendContainerPadding

            Repeater {
                id: repeater

                delegate: FilesToSendDelegate {
                    anchors.verticalCenter: filesToSendContainerRow.verticalCenter

                    width: JamiTheme.filesToSendDelegateWidth
                    height: JamiTheme.filesToSendDelegateHeight

                    onRemoveFileButtonClicked: {
                        filesToSendListModel.removeFromPending(index)
                    }
                }
                model: FilesToSendListModel {
                    id: filesToSendListModel
                }
            }
        }
    }
}
