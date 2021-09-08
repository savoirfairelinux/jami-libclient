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
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import net.jami.Constants 1.1
import net.jami.Models 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    property real margin: 5

    signal removeFileButtonClicked(int index)

    radius: JamiTheme.filesToSendDelegateRadius

    color: JamiTheme.messageInBgColor

    ColumnLayout {
        id: delegateFileWrapperColumnLayout

        anchors.fill: parent

        spacing: 0

        visible: !IsImage

        ResponsiveImage {
            id: fileIcon

            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.topMargin: margin
            Layout.leftMargin: margin

            visible: !IsImage

            source: JamiResources.file_black_24dp_svg
        }

        Text {
            id: fileName

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.leftMargin: margin
            Layout.preferredWidth: root.width - margin * 2

            visible: !IsImage

            font.pointSize: JamiTheme.filesToSendDelegateFontPointSize

            text: FileName
            elide: Text.ElideMiddle
        }

        Text {
            id: fileSize

            Layout.alignment: Qt.AlignBottom
            Layout.leftMargin: margin
            Layout.bottomMargin: margin
            Layout.preferredWidth: root.width - margin * 2

            visible: !IsImage

            font.pointSize: JamiTheme.filesToSendDelegateFontPointSize

            text: FileSize
            elide: Text.ElideMiddle
        }
    }

    AnimatedImage {
        id: name

        anchors.fill: parent

        asynchronous: true
        fillMode: Image.PreserveAspectCrop
        source: {
            if (!IsImage)
                return ""

            // :/ -> resource url for test purposes
            var sourceUrl = FilePath
            if (!sourceUrl.startsWith(":/"))
                return JamiQmlUtils.qmlFilePrefix + sourceUrl
            else
                return "qrc" + sourceUrl
        }

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: root.width
                height: root.height
                radius: JamiTheme.filesToSendDelegateRadius
            }
        }
    }

    PushButton {
        id: removeFileButton

        anchors.right: root.right
        anchors.rightMargin: -margin
        anchors.top: root.top
        anchors.topMargin: -margin

        radius: margin
        preferredSize: JamiTheme.filesToSendDelegateButtonSize

        toolTipText: JamiStrings.remove

        source: JamiResources.cross_black_24dp_svg

        normalColor:  JamiTheme.removeFileButtonColor
        hoveredColor: JamiTheme.lightGrey_
        imageColor: JamiTheme.textColor

        onClicked: root.removeFileButtonClicked(index)
    }
}
