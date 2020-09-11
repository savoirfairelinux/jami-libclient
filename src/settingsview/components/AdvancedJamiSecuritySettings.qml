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
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import Qt.labs.platform 1.1
import "../../commoncomponents"
import "../../constant"

ColumnLayout {
    id: root

    property int itemWidth

    function updateSecurityAccountInfos() {
        btnCACert.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_TLS_CertificateListFile()))
        btnCACert.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())
        btnUserCert.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_TLS_CertificateFile()))
        btnUserCert.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())
        btnPrivateKey.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile()))
        btnPrivateKey.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())
    }

    function changeFileCACert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileCACert(url)
            btnCACert.setText(UtilsAdapter.toFileInfoName(url))
        }
    }

    function changeFileUserCert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileUserCert(url)
            btnUserCert.setText(UtilsAdapter.toFileInfoName(url))
        }
    }

    function changeFilePrivateKey(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FilePrivateKey(url)
            btnPrivateKey.setText(UtilsAdapter.toFileInfoName(url))
        }
    }

    JamiFileDialog {
        id: caCert_Dialog

        property string oldPath : SettingsAdapter.getAccountConfig_TLS_CertificateListFile()
        property string openPath : oldPath === "" ? (UtilsAdapter.getCurrentPath() + "/ringtones/") : (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectCACert
        folder: openPath
        nameFilters: [qsTr("Certificate File") + " (*.crt)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())
            changeFileCACert(url)
        }
    }

    JamiFileDialog {
        id: userCert_Dialog

        property string oldPath : SettingsAdapter.getAccountConfig_TLS_CertificateFile()
        property string openPath : oldPath === "" ? (UtilsAdapter.getCurrentPath() + "/ringtones/") : (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectUserCert
        folder: openPath
        nameFilters: [qsTr("Certificate File") + " (*.crt)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())
            changeFileUserCert(url)
        }
    }

    JamiFileDialog {
        id: privateKey_Dialog

        property string oldPath : {
            return SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile()
        }
        property string openPath : oldPath === "" ? (UtilsAdapter.getCurrentPath() + "/ringtones/") : (UtilsAdapter.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: JamiStrings.selectPrivateKey
        folder: openPath
        nameFilters: [qsTr("Key File") + " (*.key)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = UtilsAdapter.getAbsPath(file.toString())
            changeFilePrivateKey(url)
        }
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
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.caCertificate
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: caCert_Dialog.open()
        }

        SettingMaterialButton {
            id: btnUserCert
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.userCertificate
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: userCert_Dialog.open()
        }

        SettingMaterialButton {
            id: btnPrivateKey
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.privateKey
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: privateKey_Dialog.open()
        }

        SettingsMaterialLineEdit {
            id: lineEditCertPassword

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.privateKeyPassword
        }
    }
}