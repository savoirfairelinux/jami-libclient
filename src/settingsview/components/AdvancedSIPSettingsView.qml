/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.3
import Qt.labs.platform 1.1
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    function updateAccountInfoDisplayedAdvanceSIP(){
        // Call Settings
        checkBoxAutoAnswerSIP.checked = SettingsAdapter.getAccountConfig_AutoAnswer()
        checkBoxCustomRingtoneSIP.checked = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()

        // security
        btnSIPCACert.enabled = SettingsAdapter.getAccountConfig_TLS_Enable()
        btnSIPUserCert.enabled = SettingsAdapter.getAccountConfig_TLS_Enable()
        btnSIPPrivateKey.enabled = SettingsAdapter.getAccountConfig_TLS_Enable()
        lineEditSIPCertPassword.enabled = SettingsAdapter.getAccountConfig_TLS_Enable()
        enableSDESToggle.enabled = SettingsAdapter.getAccountConfig_SRTP_Enabled()
        fallbackRTPToggle.enabled = SettingsAdapter.getAccountConfig_SRTP_Enabled()

        btnSIPCACert.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_TLS_CertificateListFile())
        btnSIPUserCert.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_TLS_CertificateFile())
        btnSIPPrivateKey.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile())
        lineEditSIPCertPassword.text = SettingsAdapter.getAccountConfig_TLS_Password()

        encryptMediaStreamsToggle.checked = SettingsAdapter.getAccountConfig_SRTP_Enabled()
        enableSDESToggle.checked = (ClientWrapper.SettingsAdapter.getAccountConfig_SRTP_KeyExchange()  === Account.KeyExchangeProtocol.SDES)
        fallbackRTPToggle.checked = SettingsAdapter.getAccountConfig_SRTP_RtpFallback()
        encryptNegotitationToggle.checked = SettingsAdapter.getAccountConfig_TLS_Enable()
        verifyIncomingCertificatesServerToogle.checked = SettingsAdapter.getAccountConfig_TLS_VerifyServer()
        verifyIncomingCertificatesClientToogle.checked = SettingsAdapter.getAccountConfig_TLS_VerifyClient()
        requireCeritificateForTLSIncomingToggle.checked = SettingsAdapter.getAccountConfig_TLS_RequireClientCertificate()

        var method = SettingsAdapter.getAccountConfig_TLS_Method_inInt()
        tlsProtocolComboBox.currentIndex = method

        outgoingTLSServerNameLineEdit.text = SettingsAdapter.getAccountConfig_TLS_Servername()
        negotiationTimeoutSpinBox.value = SettingsAdapter.getAccountConfig_TLS_NegotiationTimeoutSec()

        // Connectivity
        checkBoxUPnPSIP.checked = SettingsAdapter.getAccountConfig_UpnpEnabled()
        checkBoxTurnEnableSIP.checked = SettingsAdapter.getAccountConfig_TURN_Enabled()
        lineEditTurnAddressSIP.text = SettingsAdapter.getAccountConfig_TURN_Server()
        lineEditTurnUsernameSIP.text = SettingsAdapter.getAccountConfig_TURN_Username()
        lineEditTurnPsswdSIP.text = SettingsAdapter.getAccountConfig_TURN_Password()
        lineEditTurnRealmSIP.text = SettingsAdapter.getAccountConfig_TURN_Realm()
        lineEditTurnAddressSIP.enabled = SettingsAdapter.getAccountConfig_TURN_Enabled()
        lineEditTurnUsernameSIP.enabled = SettingsAdapter.getAccountConfig_TURN_Enabled()
        lineEditTurnPsswdSIP.enabled = SettingsAdapter.getAccountConfig_TURN_Enabled()
        lineEditTurnRealmSIP.enabled = SettingsAdapter.getAccountConfig_TURN_Enabled()

        checkBoxSTUNEnableSIP.checked = SettingsAdapter.getAccountConfig_STUN_Enabled()
        lineEditSTUNAddressSIP.text = SettingsAdapter.getAccountConfig_STUN_Server()
        lineEditSTUNAddressSIP.enabled = SettingsAdapter.getAccountConfig_STUN_Enabled()

        registrationExpireTimeoutSpinBox.value = SettingsAdapter.getAccountConfig_Registration_Expire()
        networkInterfaceSpinBox.value = SettingsAdapter.getAccountConfig_Localport()

        // published address
        checkBoxCustomAddressPort.checked = SettingsAdapter.getAccountConfig_PublishedSameAsLocal()
        lineEditSIPCustomAddress.text = SettingsAdapter.getAccountConfig_PublishedAddress()
        customPortSIPSpinBox.value = SettingsAdapter.getAccountConfig_PublishedPort()

        // codecs
        videoCheckBoxSIP.checked = SettingsAdapter.getAccountConfig_Video_Enabled()
        updateAudioCodecs()
        updateVideoCodecs()
        btnRingtoneSIP.enabled = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()
        btnRingtoneSIP.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_Ringtone_RingtonePath())
        lineEditSTUNAddressSIP.enabled = SettingsAdapter.getAccountConfig_STUN_Enabled()

        // SDP session negotiation ports
        audioRTPMinPortSpinBox.value = SettingsAdapter.getAccountConfig_Audio_AudioPortMin()
        audioRTPMaxPortSpinBox.value = SettingsAdapter.getAccountConfig_Audio_AudioPortMax()
        videoRTPMinPortSpinBox.value = SettingsAdapter.getAccountConfig_Video_VideoPortMin()
        videoRTPMaxPortSpinBox.value = SettingsAdapter.getAccountConfig_Video_VideoPortMax()

        // voicemail
        lineEditVoiceMailDialCode.text = SettingsAdapter.getAccountConfig_Mailbox()
    }

    function updateAudioCodecs(){
        audioListWidgetSIP.model.layoutAboutToBeChanged()
        audioListWidgetSIP.model.dataChanged(audioListWidgetSIP.model.index(0, 0),
                                     audioListWidgetSIP.model.index(audioListWidgetSIP.model.rowCount() - 1, 0))
        audioListWidgetSIP.model.layoutChanged()
    }

    function updateVideoCodecs(){
        videoListWidgetSIP.model.layoutAboutToBeChanged()
        videoListWidgetSIP.model.dataChanged(videoListWidgetSIP.model.index(0, 0),
                                     videoListWidgetSIP.model.index(videoListWidgetSIP.model.rowCount() - 1, 0))
        videoListWidgetSIP.model.layoutChanged()
    }

    function decreaseAudioCodecPriority(){
        var index = audioListWidgetSIP.currentIndex
        var codecId = audioListWidgetSIP.model.data(audioListWidgetSIP.model.index(index,0), AudioCodecListModel.AudioCodecID)

       SettingsAdapter.decreaseAudioCodecPriority(codecId)
        audioListWidgetSIP.currentIndex = index + 1
        updateAudioCodecs()
    }

    function increaseAudioCodecPriority(){
        var index = audioListWidgetSIP.currentIndex
        var codecId = audioListWidgetSIP.model.data(audioListWidgetSIP.model.index(index,0), AudioCodecListModel.AudioCodecID)

       SettingsAdapter.increaseAudioCodecPriority(codecId)
        audioListWidgetSIP.currentIndex = index - 1
        updateAudioCodecs()
    }

    function decreaseVideoCodecPriority(){
        var index = videoListWidgetSIP.currentIndex
        var codecId = videoListWidgetSIP.model.data(videoListWidgetSIP.model.index(index,0), VideoCodecListModel.VideoCodecID)

       SettingsAdapter.decreaseVideoCodecPriority(codecId)
        videoListWidgetSIP.currentIndex = index + 1
        updateVideoCodecs()
    }

    function increaseVideoCodecPriority(){
        var index = videoListWidgetSIP.currentIndex
        var codecId = videoListWidgetSIP.model.data(videoListWidgetSIP.model.index(index,0), VideoCodecListModel.VideoCodecID)

       SettingsAdapter.increaseVideoCodecPriority(codecId)
        videoListWidgetSIP.currentIndex = index - 1
        updateVideoCodecs()
    }

    // slots
    function audioRTPMinPortSpinBoxEditFinished(value){
        if (ClientWrapper.SettingsAdapter.getAccountConfig_Audio_AudioPortMax() < value) {
            audioRTPMinPortSpinBox.value = SettingsAdapter.getAccountConfig_Audio_AudioPortMin()
            return
        }
       SettingsAdapter.audioRTPMinPortSpinBoxEditFinished(value)
    }

    function audioRTPMaxPortSpinBoxEditFinished(value){
        if (value <SettingsAdapter.getAccountConfig_Audio_AudioPortMin()) {
            audioRTPMaxPortSpinBox.value = SettingsAdapter.getAccountConfig_Audio_AudioPortMax()
            return
        }
       SettingsAdapter.audioRTPMaxPortSpinBoxEditFinished(value)
    }

    function videoRTPMinPortSpinBoxEditFinished(value){
        if (ClientWrapper.SettingsAdapter.getAccountConfig_Video_VideoPortMax() < value) {
            videoRTPMinPortSpinBox.value = SettingsAdapter.getAccountConfig_Video_VideoPortMin()
            return
        }
       SettingsAdapter.videoRTPMinPortSpinBoxEditFinished(value)
    }

    function videoRTPMaxPortSpinBoxEditFinished(value){
        if (value <SettingsAdapter.getAccountConfig_Video_VideoPortMin()) {
            videoRTPMinPortSpinBox.value = SettingsAdapter.getAccountConfig_Video_VideoPortMin()
            return
        }
       SettingsAdapter.videoRTPMaxPortSpinBoxEditFinished(value)
    }


    function changeRingtonePath(url){
        if(url.length !== 0) {
           SettingsAdapter.set_RingtonePath(url)
            btnRingtoneSIP.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        } else if (ClientWrapper.SettingsAdapter.getAccountConfig_Ringtone_RingtonePath().length === 0){
            btnRingtoneSIP.text = qsTr("Add a custom ringtone")
        }
    }

    function changeFileCACert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileCACert(url)
            btnSIPCACert.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        }
    }

    function changeFileUserCert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileUserCert(url)
            btnSIPUserCert.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        }
    }

    function changeFilePrivateKey(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FilePrivateKey(url)
            btnSIPPrivateKey.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        }
    }

    JamiFileDialog {
        id: ringtonePath_Dialog_SIP

        property string oldPath : SettingsAdapter.getAccountConfig_Ringtone_RingtonePath()
        property string openPath : oldPath === "" ? (ClientWrapper.utilsAdaptor.getCurrentPath() + "/ringtones/") : (ClientWrapper.utilsAdaptor.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: qsTr("Select a new ringtone")
        folder: openPath

        nameFilters: [qsTr("Audio Files") + " (*.wav *.ogg *.opus *.mp3 *.aiff *.wma)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = ClientWrapper.utilsAdaptor.getAbsPath(file.toString())
            changeRingtonePath(url)
        }
    }

    JamiFileDialog {
        id: caCert_Dialog_SIP

        property string oldPath : SettingsAdapter.getAccountConfig_TLS_CertificateListFile()
        property string openPath : oldPath === "" ? (ClientWrapper.utilsAdaptor.getCurrentPath() + "/ringtones/") : (ClientWrapper.utilsAdaptor.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: qsTr("Select a CA certificate")
        folder: openPath
        nameFilters: [qsTr("Certificate File") + " (*.crt)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = ClientWrapper.utilsAdaptor.getAbsPath(file.toString())
            changeFileCACert(url)
        }
    }

    JamiFileDialog {
        id: userCert_Dialog_SIP

        property string oldPath : SettingsAdapter.getAccountConfig_TLS_CertificateFile()
        property string openPath : oldPath === "" ? (ClientWrapper.utilsAdaptor.getCurrentPath() + "/ringtones/") : (ClientWrapper.utilsAdaptor.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: qsTr("Select a user certificate")
        folder: openPath
        nameFilters: [qsTr("Certificate File") + " (*.crt)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = ClientWrapper.utilsAdaptor.getAbsPath(file.toString())
            changeFileUserCert(url)
        }
    }

    JamiFileDialog {
        id: privateKey_Dialog_SIP

        property string oldPath : SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile()
        property string openPath : oldPath === "" ? (ClientWrapper.utilsAdaptor.getCurrentPath() + "/ringtones/") : (ClientWrapper.utilsAdaptor.toFileAbsolutepath(oldPath))

        mode: JamiFileDialog.OpenFile
        title: qsTr("Select a private key")
        folder: openPath
        nameFilters: [qsTr("Key File") + " (*.key)", qsTr(
                "All files") + " (*)"]

        onAccepted: {
            var url = ClientWrapper.utilsAdaptor.getAbsPath(file.toString())
            changeFilePrivateKey(url)
        }
    }

    // call setting section
    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        ElidedTextLabel {
            Layout.fillWidth: true

            eText: qsTr("Call Settings")
            fontSize: JamiTheme.headerFontSize
            maxWidth: width
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: checkBoxAutoAnswerSIP

                labelText: qsTr("Auto Answer Calls")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setAutoAnswerCalls(checked)
                }
            }

            ToggleSwitch {
                id: checkBoxCustomRingtoneSIP

                labelText: qsTr("Enable Custom Ringtone")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setEnableRingtone(checked)
                    btnRingtoneSIP.enabled = checked
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    text: qsTr("Select Custom Ringtone")
                    elide: Text.ElideRight
                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true
                    verticalAlignment: Text.AlignVCenter
                }

                MaterialButton {
                    id: btnRingtoneSIP

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: itemWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        ringtonePath_Dialog_SIP.open()
                    }
                }
            }
        }
    }

    // voice mail section
    ColumnLayout {
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Voicemail")
            fontSize: JamiTheme.headerFontSize
            maxWidth: width
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: JamiTheme.preferredFieldHeight
            Layout.leftMargin: JamiTheme.preferredMarginSize

            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Voicemail Dial Code")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: lineEditVoiceMailDialCode

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.lineEditVoiceMailDialCodeEditFinished(text)
                }
            }
        }
    }

    // security section
    ColumnLayout {
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Security")
            fontSize: JamiTheme.headerFontSize
            maxWidth: width
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: encryptMediaStreamsToggle

                labelText: qsTr("Encrypt Media Streams (SRTP)")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseSRTP(checked)
                    enableSDESToggle.enabled = checked
                    fallbackRTPToggle.enabled = checked
                }
            }

            ToggleSwitch {
                id: enableSDESToggle

                labelText: qsTr("Enable SDES(Key Exchange)")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseSDES(checked)
                }
            }

            ToggleSwitch {
                id: fallbackRTPToggle

                labelText: qsTr("Can Fallback on RTP")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseRTPFallback(checked)
                }
            }

            ToggleSwitch {
                id: encryptNegotitationToggle

                labelText: qsTr("Encrypt Negotiation (TLS)")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseTLS(checked)
                    btnSIPCACert.enabled = checked
                    btnSIPUserCert.enabled = checked
                    btnSIPPrivateKey.enabled = checked
                    lineEditSIPCertPassword.enabled = checked
                }
            }

            GridLayout {
                Layout.fillWidth: true

                rows: 4
                columns: 2

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("CA Certificate")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: width
                }

                MaterialButton {
                    id: btnSIPCACert

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: itemWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        caCert_Dialog_SIP.open()
                    }
                }

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("User Certificate")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: width
                }

                MaterialButton {
                    id: btnSIPUserCert

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: itemWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        userCert_Dialog_SIP.open()
                    }
                }

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Private Key")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: width
                }

                MaterialButton {
                    id: btnSIPPrivateKey

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: itemWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        privateKey_Dialog_SIP.open()
                    }
                }

                // Private key password
                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Private Key Password")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: width
                }

                MaterialLineEdit {
                    id: lineEditSIPCertPassword

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: itemWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    echoMode: TextInput.Password

                    onEditingFinished: {
                       SettingsAdapter.lineEditSIPCertPasswordLineEditTextChanged(text)
                    }
                }
            }

            ToggleSwitch {
                id: verifyIncomingCertificatesServerToogle

                labelText: qsTr("Verify Certificates (Server Side)")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setVerifyCertificatesServer(checked)
                }
            }

            ToggleSwitch {
                id: verifyIncomingCertificatesClientToogle

                labelText: qsTr("Verify Certificates (Client Side)")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setVerifyCertificatesClient(checked)
                }
            }

            ToggleSwitch {
                id: requireCeritificateForTLSIncomingToggle

                labelText: qsTr("TLS Connections Require Certificate")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setRequireCertificatesIncomingTLS(checked)
                }
            }

            GridLayout {
                Layout.fillWidth: true

                rows: 3
                columns: 2

                Text {
                    Layout.fillWidth: true
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    text: qsTr("TLS Protocol Method")
                    elide: Text.ElideRight
                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true
                    verticalAlignment: Text.AlignVCenter
                }

                SettingParaCombobox {
                    id: tlsProtocolComboBox

                    Layout.preferredWidth: itemWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    textRole: "textDisplay"

                    model: ListModel {
                        ListElement{textDisplay: "Default"; firstArg: "Default"; secondArg: 0}
                        ListElement{textDisplay: "TLSv1"; firstArg: "TLSv1"; secondArg: 1}
                        ListElement{textDisplay: "TLSv1.1"; firstArg: "TLSv1.1"; secondArg: 2}
                        ListElement{textDisplay: "TLSv1.2"; firstArg: "TLSv1.2"; secondArg: 3}
                    }

                    onActivated: {
                        var indexOfOption = tlsProtocolComboBox.model.get(index).secondArg
                       SettingsAdapter.tlsProtocolComboBoxIndexChanged(parseInt(indexOfOption))
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    text: qsTr("Outgoing TLS Server Name")
                    elide: Text.ElideRight
                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true
                    verticalAlignment: Text.AlignVCenter
                }

                MaterialLineEdit {
                    id: outgoingTLSServerNameLineEdit

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: itemWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.outgoingTLSServerNameLineEditTextChanged(text)
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.rightMargin: JamiTheme.preferredMarginSize
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    text: qsTr("Negotiation Timeout (seconds)")
                    elide: Text.ElideRight
                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true
                    verticalAlignment: Text.AlignVCenter
                }

                SpinBox {
                    id: negotiationTimeoutSpinBox

                    Layout.preferredWidth: itemWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    from: 0
                    to: 3000
                    stepSize: 1

                    up.indicator.width: (width < 200) ? (width / 5) : 40
                    down.indicator.width: (width < 200) ? (width / 5) : 40

                    onValueModified: {
                       SettingsAdapter.negotiationTimeoutSpinBoxValueChanged(value)
                    }
                }
            }
        }
    }

    // connectivity section
    ColumnLayout {
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Connectivity")
            fontSize: JamiTheme.headerFontSize
            maxWidth: width
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            rows: 9
            columns: 2

            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Registration Expire Timeout (seconds)")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            SpinBox {
                id: registrationExpireTimeoutSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.alignment: Qt.AlignCenter

                font.pointSize: JamiTheme.buttonFontSize
                font.kerning: true

                from: 0
                to: 3000
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                   SettingsAdapter.registrationTimeoutSpinBoxValueChanged(value)
                }
            }

            // 2nd row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Newtwork interface")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            SpinBox {
                id: networkInterfaceSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.alignment: Qt.AlignCenter

                font.pointSize: JamiTheme.buttonFontSize
                font.kerning: true

                from: 0
                to: 65536
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                   SettingsAdapter.networkInterfaceSpinBoxValueChanged(value)
                }
            }

            // 3rd row
            ToggleSwitch {
                id: checkBoxUPnPSIP

                Layout.columnSpan: 2

                labelText: qsTr("Use UPnP")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseUPnP(checked)
                }
            }

            // 4th row
            ToggleSwitch {
                id: checkBoxTurnEnableSIP

                Layout.columnSpan: 2

                labelText: qsTr("Use TURN")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseTURN(checked)
                    lineEditTurnAddressSIP.enabled = checked
                    lineEditTurnUsernameSIP.enabled = checked
                    lineEditTurnPsswdSIP.enabled = checked
                    lineEditTurnRealmSIP.enabled = checked
                }
            }

            // 5th row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("TURN Address")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: lineEditTurnAddressSIP

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.setTURNAddress(text)
                }
            }

            // 6th row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("TURN Username")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: lineEditTurnUsernameSIP

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.setTURNUsername(text)
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("TURN Password")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: lineEditTurnPsswdSIP

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                echoMode: TextInput.Password

                onEditingFinished: {
                   SettingsAdapter.setTURNPassword(text)
                }
            }

            // 8th row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("TURN Realm")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: lineEditTurnRealmSIP

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.setTURNRealm(text)
                }
            }

            // 9th row
            ToggleSwitch {
                id: checkBoxSTUNEnableSIP

                labelText: qsTr("Use STUN")
                fontPointSize: JamiTheme.settingsFontSize

                Layout.columnSpan: 2

                onSwitchToggled: {
                   SettingsAdapter.setUseSTUN(checked)
                    lineEditSTUNAddressSIP.enabled = checked
                }
            }

            // 10th row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("STUN Address")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            MaterialLineEdit {
                id: lineEditSTUNAddressSIP

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.setSTUNAddress(text)
                }
            }
        }
    }

    // public address section
    ColumnLayout {
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            text: qsTr("Public Address")
            fontSize: JamiTheme.headerFontSize
            maxWidth: width
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            rows: 3
            columns: 2

            // 1st row
            ToggleSwitch {
                id: checkBoxCustomAddressPort

                labelText: qsTr("Use Custom Address/Port")
                fontPointSize: JamiTheme.settingsFontSize

                Layout.columnSpan: 2

                onSwitchToggled: {
                   SettingsAdapter.setUseCustomAddressAndPort(checked)
                    lineEditSIPCustomAddress.enabled = checked
                    customPortSIPSpinBox.enabled = checked
                }
            }

            //2nd row
            ElidedTextLabel {
                Layout.leftMargin: JamiTheme.preferredMarginSize
                Layout.fillWidth: true
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Address")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: width
            }

            MaterialLineEdit {
                id: lineEditSIPCustomAddress

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: itemWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.lineEditSIPCustomAddressLineEditTextChanged(text)
                }
            }

            //3rd row
            ElidedTextLabel {
                Layout.leftMargin: JamiTheme.preferredMarginSize
                Layout.fillWidth: true
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Port")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: width
            }

            SpinBox {
                id: customPortSIPSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.alignment: Qt.AlignCenter

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                from: 0
                to: 65535
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                   SettingsAdapter.customPortSIPSpinBoxValueChanged(value)
                }
            }
        }
    }

    // media section
    ColumnLayout {
        Layout.fillWidth: true

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            text: qsTr("Media")

            font.pointSize: JamiTheme.headerFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: videoCheckBoxSIP

                labelText: qsTr("Enable Video")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setVideoState(checked)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            maxWidth: width
                            eText:  qsTr("Video Codecs")
                            fontSize: JamiTheme.settingsFontSize
                        }

                        HoverableButtonTextItem {
                            id: videoDownPushButtonSIP

                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24

                            radius: height / 2

                            source: "qrc:/images/icons/round-arrow_drop_down-24px.svg"

                            onClicked: {
                                decreaseVideoCodecPriority()
                            }
                        }

                        HoverableButtonTextItem {
                            id: videoUpPushButtonSIP

                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24

                            radius: height / 2

                            source: "qrc:/images/icons/round-arrow_drop_up-24px.svg"

                            onClicked: {
                                increaseVideoCodecPriority()
                            }
                        }
                    }

                    ListViewJami {
                        id: videoListWidgetSIP

                        Layout.fillWidth: true
                        Layout.preferredHeight: 190

                        model: VideoCodecListModel{}

                        delegate: VideoCodecDelegate {
                            id: videoCodecDelegate

                            width: videoListWidgetSIP.width
                            height: videoListWidgetSIP.height / 4

                            videoCodecName : VideoCodecName
                            isEnabled : IsEnabled
                            videoCodecId: VideoCodecID

                            onClicked: {
                                videoListWidget.currentIndex = index
                            }

                            onVideoCodecStateChange:{
                               SettingsAdapter.videoCodecsStateChange(idToSet , isToBeEnabled)
                                updateVideoCodecs()
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: JamiTheme.preferredMarginSize / 2

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        ElidedTextLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight

                            maxWidth: width
                            eText:  qsTr("Audio Codecs")
                            fontSize: JamiTheme.settingsFontSize
                        }

                        HoverableButtonTextItem {
                            id: audioDownPushButtonSIP

                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24

                            radius: height / 2

                            source: "qrc:/images/icons/round-arrow_drop_down-24px.svg"

                            onClicked: {
                                decreaseAudioCodecPriority()
                            }
                        }

                        HoverableButtonTextItem {
                            id: audioUpPushButtonSIP

                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24

                            radius: height / 2

                            source: "qrc:/images/icons/round-arrow_drop_up-24px.svg"

                            onClicked: {
                                increaseAudioCodecPriority()
                            }
                        }
                    }

                    ListViewJami {
                        id: audioListWidgetSIP

                        Layout.fillWidth: true
                        Layout.preferredHeight: 190

                        model: AudioCodecListModel{}

                        delegate: AudioCodecDelegate {
                            id: audioCodecDelegate

                            width: audioListWidgetSIP.width
                            height: audioListWidgetSIP.height / 4

                            layer.mipmap: false
                            clip: true

                            audioCodecName : AudioCodecName
                            isEnabled : IsEnabled
                            audioCodecId: AudioCodecID
                            samplerRate: Samplerate

                            onClicked: {
                                audioListWidget.currentIndex = index
                            }

                            onAudioCodecStateChange:{
                               SettingsAdapter.audioCodecsStateChange(idToSet , isToBeEnabled)
                                updateAudioCodecs()
                            }
                        }
                    }
                }
            }
        }
    }

    // SDP Session
    ColumnLayout {
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("SDP Session Negotiation (ICE Fallback)")
            fontSize: JamiTheme.headerFontSize
            maxWidth: width
        }

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.leftMargin: JamiTheme.preferredMarginSize

            eText: qsTr("Only used during negotiation in case ICE is not supported")
            fontSize: JamiTheme.settingsFontSize
            maxWidth: width
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            rows: 4
            columns: 2

            // 1st row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Audio RTP Min Port")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            SpinBox {
                id:audioRTPMinPortSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                from: 0
                to: 65535
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                    audioRTPMinPortSpinBoxEditFinished(value)
                }
            }

            // 2nd row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Audio RTP Max Port")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            SpinBox {
                id:audioRTPMaxPortSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                from: 0
                to: 65535
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                    audioRTPMaxPortSpinBoxEditFinished(value)
                }
            }

            // 3rd row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Video RTP Min Port")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            SpinBox {
                id:videoRTPMinPortSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                from: 0
                to: 65535
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                    videoRTPMinPortSpinBoxEditFinished(value)
                }
            }

            // 4th row
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: JamiTheme.preferredMarginSize
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                text: qsTr("Video RTP Max Port")
                elide: Text.ElideRight
                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true
                verticalAlignment: Text.AlignVCenter
            }

            SpinBox {
                id:videoRTPMaxPortSpinBox

                Layout.preferredWidth: itemWidth
                Layout.preferredHeight: JamiTheme.preferredFieldHeight

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                from: 0
                to: 65535
                stepSize: 1

                up.indicator.width: (width < 200) ? (width / 5) : 40
                down.indicator.width: (width < 200) ? (width / 5) : 40

                onValueModified: {
                    videoRTPMaxPortSpinBoxEditFinished(value)
                }
            }
        }
    }
}
