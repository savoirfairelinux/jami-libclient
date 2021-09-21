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
import Qt5Compat.GraphicalEffects

import net.jami.Constants 1.1
import net.jami.Models 1.1

import "commoncomponents"

ApplicationWindow {
    id: root

    property bool connectionFailed: false
    property int preferredMargin: 15

    title: "Jami"

    width: 600
    height: 500
    minimumWidth: 600
    minimumHeight: 500

    visible: true

    TextMetrics {
        id: textMetrics
    }

    function getTextBoundingRect(font, text) {
        textMetrics.font = font
        textMetrics.text = text

        return textMetrics.boundingRect
    }

    ResponsiveImage {
        id: jamiLogoImage

        anchors.fill: parent

        source: JamiResources.logo_jami_standard_coul_svg
    }

    Popup {
        id: popup

        // center in parent
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)

        modal: true
        visible: false
        closePolicy: Popup.NoAutoClose

        contentItem: Rectangle {
            id: contentRect

            implicitHeight: daemonReconnectPopupColumnLayout.implicitHeight + 50

            ColumnLayout {
                id: daemonReconnectPopupColumnLayout

                anchors.fill: parent

                spacing: 0

                Text {
                    id: daemonReconnectPopupTextLabel

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    Layout.topMargin: preferredMargin

                    text: connectionFailed ?
                              qsTr("Could not re-connect to the Jami daemon (jamid).\nJami will now quit.") :
                              qsTr("Trying to reconnect to the Jami daemon (jamid)…")
                    font.pointSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    Component.onCompleted: {
                        contentRect.implicitWidth = getTextBoundingRect(
                                    font, text).width + 100
                    }
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

                Button {
                    id: btnOk

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    Layout.preferredWidth: 128
                    Layout.preferredHeight: 32
                    Layout.bottomMargin: preferredMargin
                    visible: connectionFailed

                    property color hoveredColor: "#0e81c5"
                    property color pressedColor: "#273261"
                    property color normalColor: "#00aaff"

                    contentItem: Item {
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"

                            Text {
                                id: buttonText

                                anchors.centerIn: parent

                                width: {
                                    return (parent.width / 2 - 18) * 2
                                }

                                text: qsTr("Ok")

                                color: {
                                    if (btnOk.hovered)
                                        return btnOk.hoveredColor
                                    if (btnOk.checked)
                                        return btnOk.pressedColor
                                    return btnOk.normalColor
                                }
                                font: root.font
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    onClicked: Qt.quit()

                    background: Rectangle {
                        id: backgroundRect
                        anchors.fill: parent
                        color: "transparent"
                        border.color: {
                            if (btnOk.hovered)
                                return btnOk.hoveredColor
                            if (btnOk.checked)
                                return btnOk.pressedColor
                            return btnOk.normalColor
                        }
                        radius: 4
                    }
                }
            }
        }
    }

    Connections {
        target: DBusErrorHandler

        function onShowDaemonReconnectPopup(visible) {
            if (visible)
                popup.open()
            else {
                popup.close()
                Qt.quit()
            }
        }

        function onDaemonReconnectFailed() {
            root.connectionFailed = true
        }
    }

    Component.onCompleted: {
        DBusErrorHandler.setActive(true)

        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }
}
