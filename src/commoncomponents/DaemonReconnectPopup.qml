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
import QtQuick.Layouts

import net.jami.Constants 1.1
import net.jami.Models 1.1

BaseModalDialog {
    id: root

    property bool connectionFailed: false
    property int preferredMargin: 15

    autoClose: false

    onPopupContentLoadStatusChanged: {
        if (popupContentLoadStatus === Loader.Ready) {
            root.height = Qt.binding(function() {
                return popupContentLoader.item.implicitHeight + 50
            })
            root.width = Qt.binding(function() {
                return popupContentLoader.item.implicitWidth + 50
            })
        }
    }

    popupContent: ColumnLayout {
        id: daemonReconnectPopupColumnLayout

        spacing: 0

        Text {
            id: daemonReconnectPopupTextLabel

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.topMargin: preferredMargin

            text: connectionFailed ? JamiStrings.reconnectionFailed :
                                     JamiStrings.reconnectDaemon
            font.pointSize: JamiTheme.textFontSize + 2
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: JamiTheme.textColor
        }

        AnimatedImage {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.preferredHeight: 30
            Layout.preferredWidth: 30
            Layout.bottomMargin: preferredMargin

            visible: !connectionFailed

            source: JamiResources.jami_rolling_spinner_gif

            playing: true
            paused: false
            mipmap: true
            smooth: true
            fillMode: Image.PreserveAspectFit
        }

        MaterialButton {
            id: btnOk

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            preferredWidth: JamiTheme.preferredFieldWidth / 2

            visible: connectionFailed

            text: JamiStrings.optionOk
            color: JamiTheme.buttonTintedBlue
            hoveredColor: JamiTheme.buttonTintedBlueHovered
            pressedColor: JamiTheme.buttonTintedBluePressed
            outlined: true

            onClicked: Qt.quit()
        }
    }
}
