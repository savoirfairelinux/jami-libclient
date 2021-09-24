/*
 * Copyright (C) 2021 by Savoir-faire Linux
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
import Qt5Compat.GraphicalEffects

import net.jami.Models 1.1
import net.jami.Constants 1.1

MaterialLineEdit {
    id: root

    enum NameRegistrationState {
        BLANK,
        INVALID,
        TAKEN,
        FREE,
        SEARCHING
    }

    property int nameRegistrationState: UsernameLineEdit.NameRegistrationState.BLANK
    property var __iconSource: ""

    selectByMouse: true
    font.pointSize: JamiTheme.usernameLineEditPointSize
    font.kerning: true

    Connections {
        id: registeredNameFoundConnection

        target: NameDirectory
        enabled: root.text.length !== 0

        function onRegisteredNameFound(status, address, name) {
            if (text === name) {
                switch(status) {
                case NameDirectory.LookupStatus.NOT_FOUND:
                    nameRegistrationState = UsernameLineEdit.NameRegistrationState.FREE
                    break
                case NameDirectory.LookupStatus.ERROR:
                case NameDirectory.LookupStatus.INVALID_NAME:
                case NameDirectory.LookupStatus.INVALID:
                    nameRegistrationState = UsernameLineEdit.NameRegistrationState.INVALID
                    break
                case NameDirectory.LookupStatus.SUCCESS:
                    nameRegistrationState = UsernameLineEdit.NameRegistrationState.TAKEN
                    break
                }
            }
        }
    }

    Timer {
        id: lookupTimer

        repeat: false
        interval: JamiTheme.usernameLineEditlookupInterval

        onTriggered: {
            if (text.length !== 0 && readOnly === false) {
                nameRegistrationState = UsernameLineEdit.NameRegistrationState.SEARCHING
                NameDirectory.lookupName("", text)
            } else {
                nameRegistrationState = UsernameLineEdit.NameRegistrationState.BLANK
            }
        }
    }

    ResponsiveImage {
        id: lineEditImage

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: JamiTheme.preferredMarginSize / 2

        visible: nameRegistrationState !== UsernameLineEdit.NameRegistrationState.SEARCHING
        source: nameRegistrationState === UsernameLineEdit.NameRegistrationState.SEARCHING ?
                    "" : __iconSource
        color: borderColor
    }

    AnimatedImage {
        anchors.left: lineEditImage.left
        anchors.verticalCenter: parent.verticalCenter

        width: 24
        height: 24

        source: nameRegistrationState !== UsernameLineEdit.NameRegistrationState.SEARCHING ?
                    "" : __iconSource
        playing: true
        paused: false
        fillMode: Image.PreserveAspectFit
        mipmap: true
        visible: nameRegistrationState === UsernameLineEdit.NameRegistrationState.SEARCHING
    }

    onNameRegistrationStateChanged: {
        if (!enabled)
            borderColor = "transparent"

        switch(nameRegistrationState){
        case UsernameLineEdit.NameRegistrationState.SEARCHING:
            __iconSource = JamiResources.jami_rolling_spinner_gif
            borderColor = JamiTheme.greyBorderColor
            break
        case UsernameLineEdit.NameRegistrationState.BLANK:
            __iconSource = ""
            borderColor = JamiTheme.greyBorderColor
            break
        case UsernameLineEdit.NameRegistrationState.FREE:
            __iconSource = JamiResources.round_check_circle_24dp_svg
            borderColor = "green"
            break
        case UsernameLineEdit.NameRegistrationState.INVALID:
        case UsernameLineEdit.NameRegistrationState.TAKEN:
            __iconSource = JamiResources.round_error_24dp_svg
            borderColor = "red"
            break
        }
    }

    onTextChanged: lookupTimer.restart()
}
