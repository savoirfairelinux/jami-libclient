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

import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

BaseModalDialog {
    id: root

    //Content height + margin.
    property int size: JamiTheme.qrCodeImageSize + 30

    width: size
    height: size

    backgroundColor: JamiTheme.whiteColor

    popupContentPreferredHeight: JamiTheme.qrCodeImageSize
    popupContentPreferredWidth: JamiTheme.qrCodeImageSize
    popupContent: Image {
        id: userQrImage

        smooth: false
        fillMode: Image.PreserveAspectFit
        source: "image://qrImage/account_" + CurrentAccount.id
    }
}
