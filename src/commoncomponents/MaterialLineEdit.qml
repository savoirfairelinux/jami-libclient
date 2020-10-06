/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Sébastien blin <sebastien.blin@savoirfairelinux.com>
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
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14

import "../constant"

TextField {
    enum BorderColorMode {
        NORMAL,
        SEARCHING,
        RIGHT,
        ERROR
    }

    property int fieldLayoutWidth: 256
    property int fieldLayoutHeight: 48
    property bool layoutFillwidth: false

    property int borderColorMode: MaterialLineEdit.NORMAL
    property var iconSource: ""
    property var backgroundColor: JamiTheme.rgb256(240,240,240)
    property var borderColor: "#333"

    signal imageClicked

    onBorderColorModeChanged: {
        if (!enabled)
            borderColor = "transparent"
        if (readOnly)
            iconSource = ""

        switch(borderColorMode){
        case MaterialLineEdit.SEARCHING:
            iconSource = "qrc:/images/jami_rolling_spinner.gif"
            borderColor = "#333"
            break
        case MaterialLineEdit.NORMAL:
            iconSource = ""
            borderColor = "#333"
            break
        case MaterialLineEdit.RIGHT:
            iconSource = "qrc:/images/icons/round-check_circle-24px.svg"
            borderColor = "green"
            break
        case MaterialLineEdit.ERROR:
            iconSource = "qrc:/images/icons/round-error-24px.svg"
            borderColor = "red"
            break
        }
    }

    wrapMode: Text.Wrap
    readOnly: false
    selectByMouse: true
    font.pointSize: 10
    padding: 16
    font.kerning: true
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter

    Image {
        id: lineEditImage

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 16

        width: 24
        height: 24

        visible: borderColorMode !== MaterialLineEdit.SEARCHING
        source: borderColorMode === MaterialLineEdit.SEARCHING ? "" : iconSource
        layer {
            enabled: true
            effect: ColorOverlay {
                id: overlay
                color: borderColor
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            enabled: borderColorMode === MaterialLineEdit.RIGHT

            onReleased: {
                imageClicked()
            }
        }
    }

    AnimatedImage {
        anchors.left: lineEditImage.left
        anchors.verticalCenter: parent.verticalCenter

        width: 24
        height: 24

        source: borderColorMode !== MaterialLineEdit.SEARCHING ? "" : iconSource
        playing: true
        paused: false
        fillMode: Image.PreserveAspectFit
        mipmap: true
        visible: borderColorMode === MaterialLineEdit.SEARCHING
    }

    background: Rectangle {
        anchors.fill: parent
        radius: 4
        border.color: readOnly? "transparent" : borderColor
        color: readOnly? "transparent" : backgroundColor
    }
}
