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

import net.jami.Constants 1.0

MaterialButton {
    id: root

    property bool spinnerTriggered: false
    property string spinnerTriggeredtext: value
    property string normalText: value

    animatedImageSource: spinnerTriggered ? "qrc:/images/jami_rolling_spinner.gif" : ""
    text: spinnerTriggered ? spinnerTriggeredtext : normalText
    color: !enabled ? JamiTheme.buttonTintedGreyInactive :
                      JamiTheme.wizardBlueButtons

    hoveredColor: JamiTheme.buttonTintedBlueHovered
    pressedColor: JamiTheme.buttonTintedBluePressed
}
