
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
import QtQuick.Layouts 1.14
import net.jami.Models 1.0


Dialog {
    id: userQrImageDialog

    property string accountIdStr: ClientWrapper.utilsAdaptor.getCurrAccId()

    function updateQrDialog() {
        accountIdStr = ClientWrapper.utilsAdaptor.getCurrAccId()
    }

    // When dialog is opened, trigger mainViewWindow overlay which is defined in overlay.model.
    // (model : true is necessary)
    modal: true

    //Content height + margin.
    contentHeight: userQrImage.height + 30

    Image {
        id: userQrImage

        anchors.centerIn: parent

        width: 250
        height: 250

        fillMode: Image.PreserveAspectFit
        source: "image://qrImage/account_" + accountIdStr
        sourceSize.width: 260
        sourceSize.height: 260
        mipmap: true
    }

    background: Rectangle {
        border.width: 0
        radius: 10
    }
}