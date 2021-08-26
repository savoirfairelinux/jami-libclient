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
import QtQuick.Controls 2.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    JamiFileDialog {
        id: caCert_Dialog_SIP

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
        id: userCert_Dialog_SIP

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
        id: privateKey_Dialog_SIP

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

        ToggleSwitch {
            id: encryptMediaStreamsToggle

            labelText: JamiStrings.encryptMediaStream
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.enable_SRTP

            onSwitchToggled: CurrentAccount.enable_SRTP = checked
        }

        ToggleSwitch {
            id: enableSDESToggle

            enabled: CurrentAccount.enable_SRTP

            labelText: JamiStrings.enableSDES
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.keyExchange_SRTP

            onSwitchToggled: CurrentAccount.keyExchange_SRTP = Number(checked)
        }

        ToggleSwitch {
            id: fallbackRTPToggle

            enabled: CurrentAccount.enable_SRTP

            labelText: JamiStrings.fallbackRTP
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.rtpFallback_SRTP

            onSwitchToggled: CurrentAccount.rtpFallback_SRTP = checked
        }

        ToggleSwitch {
            id: encryptNegotitationToggle

            labelText: JamiStrings.encryptNegotiation
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.enable_TLS

            onSwitchToggled: CurrentAccount.enable_TLS = checked
        }

        SettingMaterialButton {
            id: btnSIPCACert

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS

            titleField: JamiStrings.caCertificate
            source: JamiResources.round_folder_24dp_svg
            itemWidth: root.itemWidth

            textField: UtilsAdapter.toFileInfoName(CurrentAccount.certificateListFile_TLS)

            onClick: caCert_Dialog_SIP.open()
        }

        SettingMaterialButton {
            id: btnSIPUserCert

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS

            titleField: JamiStrings.userCertificate
            source: JamiResources.round_folder_24dp_svg
            itemWidth: root.itemWidth

            textField: UtilsAdapter.toFileInfoName(CurrentAccount.certificateFile_TLS)

            onClick: userCert_Dialog_SIP.open()
        }

        SettingMaterialButton {
            id: btnSIPPrivateKey

            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS

            titleField: JamiStrings.privateKey
            source: JamiResources.round_folder_24dp_svg
            itemWidth: root.itemWidth

            textField: UtilsAdapter.toFileInfoName(CurrentAccount.privateKeyFile_TLS)

            onClick: privateKey_Dialog_SIP.open()
        }

        // Private key password
        SettingsMaterialLineEdit {
            id: lineEditSIPCertPassword

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            enabled: CurrentAccount.enable_TLS

            itemWidth: root.itemWidth
            titleField: JamiStrings.privateKeyPassword

            textField: CurrentAccount.password_TLS

            onEditFinished: CurrentAccount.password_TLS = textField
        }

        ToggleSwitch {
            id: verifyIncomingCertificatesServerToggle

            labelText: JamiStrings.verifyCertificatesServer
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.verifyServer_TLS

            onSwitchToggled: CurrentAccount.verifyServer_TLS = checked
        }

        ToggleSwitch {
            id: verifyIncomingCertificatesClientToggle

            labelText: JamiStrings.verifyCertificatesClient
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.verifyClient_TLS

            onSwitchToggled: CurrentAccount.verifyClient_TLS = checked
        }

        ToggleSwitch {
            id: requireCeritificateForTLSIncomingToggle

            labelText: JamiStrings.tlsRequireConnections
            fontPointSize: JamiTheme.settingsFontSize

            checked: CurrentAccount.requireClientCertificate_TLS

            onSwitchToggled: CurrentAccount.requireClientCertificate_TLS = checked
        }

        SettingsComboBox {
            id: tlsProtocolComboBox

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            labelText: JamiStrings.tlsProtocol
            fontPointSize: JamiTheme.settingsFontSize
            comboModel: ListModel {
                ListElement{textDisplay: "Default"; firstArg: "Default"; secondArg: 0}
                ListElement{textDisplay: "TLSv1"; firstArg: "TLSv1"; secondArg: 1}
                ListElement{textDisplay: "TLSv1.1"; firstArg: "TLSv1.1"; secondArg: 2}
                ListElement{textDisplay: "TLSv1.2"; firstArg: "TLSv1.2"; secondArg: 3}
            }
            widthOfComboBox: root.itemWidth
            tipText: JamiStrings.audioDeviceSelector
            role: "textDisplay"

            modelIndex: CurrentAccount.method_TLS

            onModelIndexChanged: CurrentAccount.method_TLS =
                                 parseInt(comboModel.get(modelIndex).secondArg)
        }

        SettingsMaterialLineEdit {
            id: outgoingTLSServerNameLineEdit

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.tlsServerName

            textField: CurrentAccount.serverName_TLS

            onEditFinished: CurrentAccount.serverName_TLS = textField
        }

        SettingSpinBox {
            id: negotiationTimeoutSpinBox
            Layout.fillWidth: true

            title: JamiStrings.negotiationTimeOut
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 3000

            valueField: CurrentAccount.negotiationTimeoutSec_TLS

            onInputAcceptableChanged: {
                if (!inputAcceptable && valueField.length !== 0)
                    valueField = Qt.binding(function() { return CurrentAccount.negotiationTimeoutSec_TLS })
            }

            onNewValue: CurrentAccount.negotiationTimeoutSec_TLS = valueField
        }
    }
}
