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
        interval: 200

        onTriggered: {
            if (text.length !== 0 && readOnly === false) {
                nameRegistrationState = UsernameLineEdit.NameRegistrationState.SEARCHING
                NameDirectory.lookupName("", text)
            } else {
                nameRegistrationState = UsernameLineEdit.NameRegistrationState.BLANK
            }
        }
    }

    selectByMouse: true
    font.pointSize: 9
    font.kerning: true

    borderColorMode: {
        switch (nameRegistrationState){
        case UsernameLineEdit.NameRegistrationState.BLANK:
            return MaterialLineEdit.NORMAL
        case UsernameLineEdit.NameRegistrationState.INVALID:
        case UsernameLineEdit.NameRegistrationState.TAKEN:
            return MaterialLineEdit.ERROR
        case UsernameLineEdit.NameRegistrationState.FREE:
            return MaterialLineEdit.RIGHT
        case UsernameLineEdit.NameRegistrationState.SEARCHING:
            return MaterialLineEdit.SEARCHING
        }
    }

    onTextChanged: lookupTimer.restart()
}
