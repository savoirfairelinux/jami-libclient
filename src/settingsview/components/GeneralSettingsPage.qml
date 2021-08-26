/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Enums 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property int preferredColumnWidth : Math.min(root.width / 2 - 50, 350)
    property int contentWidth: generalSettingsColumnLayout.width
    property int preferredHeight: generalSettingsColumnLayout.implicitHeight

    color: JamiTheme.secondaryBackgroundColor

    ColumnLayout {
        id: generalSettingsColumnLayout

        anchors.horizontalCenter: root.horizontalCenter

        width: Math.min(JamiTheme.maximumWidthSettingsView, root.width)

        // system setting panel
        SystemSettings {
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            itemWidth: preferredColumnWidth
        }

        // chatview setting panel
        ChatviewSettings {
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            itemWidth: preferredColumnWidth
        }

        // file transfer setting panel
        FileTransferSettings {
            id: fileTransferSettings
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            itemWidth: preferredColumnWidth
        }

        // call recording setting panel
        RecordingSettings {
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize

            itemWidth: preferredColumnWidth
        }

        // Trouble shooting setting panel
        TroubleshootSettings {
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
            Layout.bottomMargin: JamiTheme.preferredMarginSize
            itemWidth: preferredColumnWidth
        }

        // update setting panel
        UpdateSettings {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize
            Layout.rightMargin: JamiTheme.preferredMarginSize
            Layout.bottomMargin: JamiTheme.preferredMarginSize
            visible: Qt.platform.os == "windows" ? true : false
        }
    }
}
