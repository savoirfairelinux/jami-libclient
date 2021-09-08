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

import net.jami.Constants 1.1

import "../../commoncomponents"

Row {
    id: root

    property int steps: 0
    property int currentStep: 0

    Repeater {
        model: steps

        Rectangle {
            color: {
                if (modelData === currentStep - 1)
                    return JamiTheme.accountCreationCurrentStepColor
                return JamiTheme.accountCreationOtherStepColor
            }
            radius: height / 2
            implicitHeight: 12
            implicitWidth: 12
        }
    }
}
