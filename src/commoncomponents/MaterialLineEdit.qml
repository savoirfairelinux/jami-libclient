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

import net.jami.Constants 1.0

TextField {
    id: root

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
    property var backgroundColor: JamiTheme.editBackgroundColor
    property var borderColor: JamiTheme.greyBorderColor

    property bool loseFocusWhenEnterPressed: false

    signal imageClicked

    onBorderColorModeChanged: {
        if (!enabled)
            borderColor = "transparent"
        if (readOnly)
            iconSource = ""

        switch(borderColorMode){
        case MaterialLineEdit.SEARCHING:
            iconSource = JamiResources.jami_rolling_spinner_gif
            borderColor = JamiTheme.greyBorderColor
            break
        case MaterialLineEdit.NORMAL:
            iconSource = ""
            borderColor = JamiTheme.greyBorderColor
            break
        case MaterialLineEdit.RIGHT:
            iconSource = JamiResources.round_check_circle_24dp_svg
            borderColor = "green"
            break
        case MaterialLineEdit.ERROR:
            iconSource = JamiResources.round_error_24dp_svg
            borderColor = "red"
            break
        }
    }

    wrapMode: Text.Wrap
    readOnly: false
    selectByMouse: true
    selectionColor: JamiTheme.placeHolderTextFontColor
    font.pointSize: 10
    padding: 16
    font.kerning: true
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter
    color: JamiTheme.textColor

    LineEditContextMenu {
        id: lineEditContextMenu

        lineEditObj: root
        selectOnly: readOnly
    }

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

        radius: JamiTheme.primaryRadius
        border.color: readOnly? "transparent" : borderColor
        color: readOnly? "transparent" : backgroundColor
    }

    onReleased: {
        if (event.button == Qt.RightButton)
            lineEditContextMenu.openMenuAt(event)
    }

    // Enter/Return keys intervention
    // Now, both editingFinished and accepted
    // signals will be emitted with focus set to false
    // Use editingFinished when the info is saved by focus lost
    // (since losing focus will also emit editingFinished)
    // Use accepted when the info is not saved by focus lost
    Keys.onPressed: {
        if (event.key === Qt.Key_Enter ||
                event.key === Qt.Key_Return) {
            if (loseFocusWhenEnterPressed)
                root.focus = false
            root.accepted()
            event.accepted = true;
        }
    }
}
