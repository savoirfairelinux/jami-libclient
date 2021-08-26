/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    JamiFileDialog {
        id: caCert_Dialog

        property string oldPath: CurrentAccount.certificateListFile_TLS
        property string openPath: oldPath === "" ?
                                      (UtilsAdapter.getCurrentPath() + "/ringtones/") :
                                      (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectCACert
        folder: openPath
        nameFilters: [qsTr("Certificate File") + " (*.crt)",
            qsTr("All files") + " (*)"]

        onAccepted: CurrentAccount.certificateListFile_TLS =
                    UtilsAdapter.getAbsPath(file.toString())
    }

    JamiFileDialog {
        id: userCert_Dialog

        property string oldPath: CurrentAccount.certificateFile_TLS
        property string openPath: oldPath === "" ?
                                      (UtilsAdapter.getCurrentPath() + "/ringtones/") :
                                      (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectUserCert
        folder: openPath
        nameFilters: [qsTr("Certificate File") + " (*.crt)",
            qsTr("All files") + " (*)"]

        onAccepted: CurrentAccount.certificateFile_TLS =
                    UtilsAdapter.getAbsPath(file.toString())
    }

    JamiFileDialog {
        id: privateKey_Dialog

        property string oldPath: CurrentAccount.privateKeyFile_TLS
        property string openPath: oldPath === "" ?
                                      (UtilsAdapter.getCurrentPath() + "/ringtones/") :
                                      (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectPrivateKey
        folder: openPath
        nameFilters: [qsTr("Key File") + " (*.key)",
            qsTr("All files") + " (*)"]

        onAccepted: CurrentAccount.privateKeyFile_TLS =
                    UtilsAdapter.getAbsPath(file.toString())
    }

    ElidedTextLabel {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        eText: JamiStrings.security
        fontSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        SettingMaterialButton {
            id: btnCACert

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS
            textField: UtilsAdapter.toFileInfoName(CurrentAccount.certificateListFile_TLS)
            titleField: JamiStrings.caCertificate
            source: JamiResources.round_folder_24dp_svg
            itemWidth: root.itemWidth

            onClick: caCert_Dialog.open()
        }

        SettingMaterialButton {
            id: btnUserCert

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS
            textField: UtilsAdapter.toFileInfoName(CurrentAccount.certificateFile_TLS)
            titleField: JamiStrings.userCertificate
            source: JamiResources.round_folder_24dp_svg
            itemWidth: root.itemWidth

            onClick: userCert_Dialog.open()
        }

        SettingMaterialButton {
            id: btnPrivateKey

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS
            textField: UtilsAdapter.toFileInfoName(CurrentAccount.privateKeyFile_TLS)
            titleField: JamiStrings.privateKey
            source: JamiResources.round_folder_24dp_svg
            itemWidth: root.itemWidth

            onClick: privateKey_Dialog.open()
        }

        SettingsMaterialLineEdit {
            id: lineEditCertPassword

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            itemWidth: root.itemWidth
            titleField: JamiStrings.privateKeyPassword

            textField: CurrentAccount.password_TLS

            onEditFinished: CurrentAccount.password_TLS = textField
        }
    }
}
