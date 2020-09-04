
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

Button {
    id: tintedButton


    // TintColor, color for the pixmap when button is hovered.
    property string tintColor: "white"


    // NormalPixmapSource - icons in normal state (non-toggled).
    // SelectedPixmapSource - icons once button is toggled.
    property string normalPixmapSource: ""
    property string selectedPixmapSource: ""


    // IsSelected property is to help set button current state manually.
    property bool isSelected: false


    // ButtonEntered signal is to help call overlay change its opacity
    signal buttonEntered

    function setChecked(checked) {
        isSelected = checked
        if (isSelected) {
            tintedButtonImage.source = selectedPixmapSource
        } else {
            tintedButtonImage.source = normalPixmapSource
        }
    }

    hoverEnabled: true

    background: Rectangle {
        id: tintedButtonBackground

        radius: 30
        color: "transparent"

        Image {
            id: tintedButtonImage

            anchors.centerIn: tintedButtonBackground

            height: tintedButtonBackground.height - 10
            width: tintedButtonBackground.width - 10

            source: normalPixmapSource
            fillMode: Image.PreserveAspectFit
            mipmap: true
            asynchronous: true
        }

        MouseArea {
            anchors.fill: parent

            hoverEnabled: true

            onReleased: {
                isSelected = !isSelected
                if (isSelected) {
                    tintedButtonImage.source = "image://tintedPixmap/"
                            + selectedPixmapSource.replace(
                                "qrc:/images/icons/", "") + "+" + tintColor
                } else {
                    tintedButtonImage.source = "image://tintedPixmap/" + normalPixmapSource.replace(
                                "qrc:/images/icons/", "") + "+" + tintColor
                }
                tintedButton.clicked()
            }
            onEntered: {


                // Tinted.
                if (isSelected) {
                    tintedButtonImage.source = "image://tintedPixmap/"
                            + selectedPixmapSource.replace(
                                "qrc:/images/icons/", "") + "+" + tintColor
                } else {
                    tintedButtonImage.source = "image://tintedPixmap/" + normalPixmapSource.replace(
                                "qrc:/images/icons/", "") + "+" + tintColor
                }
                tintedButton.buttonEntered()
            }
            onExited: {
                if (isSelected) {
                    tintedButtonImage.source = selectedPixmapSource
                } else {
                    tintedButtonImage.source = normalPixmapSource
                }
            }
        }
    }
}
