/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import net.jami.Constants 1.1

Popup {
    id: root

    // convient access to closePolicy
    property bool autoClose: true
    property alias backgroundColor: container.color
    property alias title: titleText.text
    property var popupContentLoader: containerSubContentLoader
    property alias popupContentLoadStatus: containerSubContentLoader.status
    property alias popupContent: containerSubContentLoader.sourceComponent
    property int popupContentPreferredHeight: 0
    property int popupContentPreferredWidth: 0

    parent: Overlay.overlay

    // center in parent
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)

    modal: true

    padding: 0

    // A popup is invisible until opened.
    visible: false
    closePolicy:  autoClose ?
                      (Popup.CloseOnEscape | Popup.CloseOnPressOutside) :
                      Popup.NoAutoClose

    Rectangle {
        id: container

        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            Text {
                id: titleText

                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.margins: text.length === 0 ? 0 : 10

                Layout.preferredHeight: text.length === 0 ? 0 : contentHeight

                font.pointSize: 12
                color: JamiTheme.textColor
            }

            Loader {
                id: containerSubContentLoader

                Layout.alignment: Qt.AlignCenter

                Layout.fillWidth: popupContentPreferredWidth === 0
                Layout.fillHeight: popupContentPreferredHeight === 0
                Layout.preferredHeight: popupContentPreferredHeight
                Layout.preferredWidth: popupContentPreferredWidth
            }
        }

        radius: JamiTheme.modalPopupRadius
        color: JamiTheme.secondaryBackgroundColor
    }

    background: Rectangle {
        color: JamiTheme.transparentColor
    }

    Overlay.modal: Rectangle {
        color: JamiTheme.transparentColor

        // Color animation for overlay when pop up is shown.
        ColorAnimation on color {
            to: JamiTheme.popupOverlayColor
            duration: 500
        }
    }

    DropShadow {
        z: -1
        width: root.width
        height: root.height
        horizontalOffset: 3.0
        verticalOffset: 3.0
        radius: container.radius * 4
        color: JamiTheme.shadowColor
        source: container
        transparentBorder: true
    }

    enter: Transition {
        NumberAnimation {
            properties: "opacity"; from: 0.0; to: 1.0
            duration: JamiTheme.shortFadeDuration
        }
    }
    exit: Transition {
        NumberAnimation {
            properties: "opacity"; from: 1.0; to: 0.0
            duration: JamiTheme.shortFadeDuration
        }
    }
}
