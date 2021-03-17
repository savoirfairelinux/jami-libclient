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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtGraphicalEffects 1.14
import Qt.labs.platform 1.1

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    function updateSecurityAccountInfos() {
        enableSDESToggle.enabled = SettingsAdapter.getAccountConfig_SRTP_Enabled()
        fallbackRTPToggle.enabled = SettingsAdapter.getAccountConfig_SRTP_Enabled()
        btnSIPCACert.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())
        btnSIPUserCert.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())
        btnSIPPrivateKey.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())
        lineEditSIPCertPassword.setEnabled(SettingsAdapter.getAccountConfig_TLS_Enable())

        btnSIPCACert.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_TLS_CertificateListFile()))
        btnSIPUserCert.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_TLS_CertificateFile()))
        btnSIPPrivateKey.setText(UtilsAdapter.toFileInfoName(SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile()))
        lineEditSIPCertPassword.setText(SettingsAdapter.getAccountConfig_TLS_Password())

        encryptMediaStreamsToggle.checked = SettingsAdapter.getAccountConfig_SRTP_Enabled()
        enableSDESToggle.checked = (SettingsAdapter.getAccountConfig_SRTP_KeyExchange()  === Account.KeyExchangeProtocol.SDES)
        fallbackRTPToggle.checked = SettingsAdapter.getAccountConfig_SRTP_RtpFallback()
        encryptNegotitationToggle.checked = SettingsAdapter.getAccountConfig_TLS_Enable()
        verifyIncomingCertificatesServerToggle.checked = SettingsAdapter.getAccountConfig_TLS_VerifyServer()
        verifyIncomingCertificatesClientToggle.checked = SettingsAdapter.getAccountConfig_TLS_VerifyClient()
        requireCeritificateForTLSIncomingToggle.checked = SettingsAdapter.getAccountConfig_TLS_RequireClientCertificate()

        var method = SettingsAdapter.getAccountConfig_TLS_Method_inInt()
        tlsProtocolComboBox.setCurrentIndex(method)

        outgoingTLSServerNameLineEdit.setText(SettingsAdapter.getAccountConfig_TLS_Servername())
        negotiationTimeoutSpinBox.setValue(SettingsAdapter.getAccountConfig_TLS_NegotiationTimeoutSec())
    }

    function changeFileCACert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileCACert(url)
            btnSIPCACert.setText(UtilsAdapter.toFileInfoName(url))
        }
    }

    function changeFileUserCert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileUserCert(url)
            btnSIPUserCert.setText(UtilsAdapter.toFileInfoName(url))
        }
    }

    function changeFilePrivateKey(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FilePrivateKey(url)
            btnSIPPrivateKey.setText(UtilsAdapter.toFileInfoName(url))
        }
    }

    JamiFileDialog {
        id: caCert_Dialog_SIP

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
        id: userCert_Dialog_SIP

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
        id: privateKey_Dialog_SIP

        property string oldPath : SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile()
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

        ToggleSwitch {
            id: encryptMediaStreamsToggle

            labelText: JamiStrings.encryptMediaStream
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseSRTP(checked)
                enableSDESToggle.enabled = checked
                fallbackRTPToggle.enabled = checked
            }
        }

        ToggleSwitch {
            id: enableSDESToggle

            labelText: JamiStrings.enableSDES
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseSDES(checked)
            }
        }

        ToggleSwitch {
            id: fallbackRTPToggle

            labelText: JamiStrings.fallbackRTP
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseRTPFallback(checked)
            }
        }

        ToggleSwitch {
            id: encryptNegotitationToggle

            labelText: JamiStrings.encryptNegotiation
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setUseTLS(checked)
                btnSIPCACert.setEnabled(checked)
                btnSIPUserCert.setEnabled(checked)
                btnSIPPrivateKey.setEnabled(checked)
                lineEditSIPCertPassword.setEnabled(checked)
            }
        }

        SettingMaterialButton {
            id: btnSIPCACert
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.caCertificate
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: caCert_Dialog_SIP.open()
        }

        SettingMaterialButton {
            id: btnSIPUserCert
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.userCertificate
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: userCert_Dialog_SIP.open()
        }

        SettingMaterialButton {
            id: btnSIPPrivateKey
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight

            titleField: JamiStrings.privateKey
            source: "qrc:/images/icons/round-folder-24px.svg"
            itemWidth: root.itemWidth
            onClick: privateKey_Dialog_SIP.open()
        }

        // Private key password
        SettingsMaterialLineEdit {
            id: lineEditSIPCertPassword

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.privateKeyPassword

            onEditFinished: SettingsAdapter.lineEditSIPCertPasswordLineEditTextChanged(textField)
        }

        ToggleSwitch {
            id: verifyIncomingCertificatesServerToggle

            labelText: JamiStrings.verifyCertificatesServer
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setVerifyCertificatesServer(checked)
            }
        }

        ToggleSwitch {
            id: verifyIncomingCertificatesClientToggle

            labelText: JamiStrings.verifyCertificatesClient
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setVerifyCertificatesClient(checked)
            }
        }

        ToggleSwitch {
            id: requireCeritificateForTLSIncomingToggle

            labelText: JamiStrings.tlsRequireConnections
            fontPointSize: JamiTheme.settingsFontSize

            onSwitchToggled: {
                SettingsAdapter.setRequireCertificatesIncomingTLS(checked)
            }
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

            onIndexChanged: {
                var indexOfOption = comboModel.get(modelIndex).secondArg
                SettingsAdapter.tlsProtocolComboBoxIndexChanged(parseInt(indexOfOption))
            }
        }

        SettingsMaterialLineEdit {
            id: outgoingTLSServerNameLineEdit

            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            itemWidth: root.itemWidth
            titleField: JamiStrings.tlsServerName

            onEditFinished: SettingsAdapter.outgoingTLSServerNameLineEditTextChanged(textField)
        }

        SettingSpinBox {
            id: negotiationTimeoutSpinBox
            Layout.fillWidth: true

            title: JamiStrings.negotiationTimeOut
            itemWidth: root.itemWidth
            bottomValue: 0
            topValue: 3000
            step: 1

            onNewValue: SettingsAdapter.negotiationTimeoutSpinBoxValueChanged(valueField)
        }
    }
}
