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
        audioCodecListModelSIP.layoutAboutToBeChanged()
        audioCodecListModelSIP.dataChanged(audioCodecListModelSIP.index(0, 0),
                                     audioCodecListModelSIP.index(audioCodecListModelSIP.rowCount() - 1, 0))
        audioCodecListModelSIP.layoutChanged()
    }

    function updateVideoCodecs(){
        videoCodecListModelSIP.layoutAboutToBeChanged()
        videoCodecListModelSIP.dataChanged(videoCodecListModelSIP.index(0, 0),
                                     videoCodecListModelSIP.index(videoCodecListModelSIP.rowCount() - 1, 0))
        videoCodecListModelSIP.layoutChanged()
    }

    function decreaseAudioCodecPriority(){
        var index = audioListWidgetSIP.currentIndex
        var codecId = audioCodecListModelSIP.data(audioCodecListModelSIP.index(index,0), AudioCodecListModel.AudioCodecID)

       SettingsAdapter.decreaseAudioCodecPriority(codecId)
        audioListWidgetSIP.currentIndex = index + 1
        updateAudioCodecs()
    }

    function increaseAudioCodecPriority(){
        var index = audioListWidgetSIP.currentIndex
        var codecId = audioCodecListModelSIP.data(audioCodecListModelSIP.index(index,0), AudioCodecListModel.AudioCodecID)

       SettingsAdapter.increaseAudioCodecPriority(codecId)
        audioListWidgetSIP.currentIndex = index - 1
        updateAudioCodecs()
    }

    function decreaseVideoCodecPriority(){
        var index = videoListWidgetSIP.currentIndex
        var codecId = videoCodecListModelSIP.data(videoCodecListModelSIP.index(index,0), VideoCodecListModel.VideoCodecID)

       SettingsAdapter.decreaseVideoCodecPriority(codecId)
        videoListWidgetSIP.currentIndex = index + 1
        updateVideoCodecs()
    }

    function increaseVideoCodecPriority(){
        var index = videoListWidgetSIP.currentIndex
        var codecId = videoCodecListModelSIP.data(videoCodecListModelSIP.index(index,0), VideoCodecListModel.VideoCodecID)

       SettingsAdapter.increaseVideoCodecPriority(codecId)
        videoListWidgetSIP.currentIndex = index - 1
        updateVideoCodecs()
    }

    VideoCodecListModel{
        id: videoCodecListModelSIP
    }

    AudioCodecListModel{
        id: audioCodecListModelSIP
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

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

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

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

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

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

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

        onRejected: {}

        onVisibleChanged: {
            if (!visible) {
                rejected()
            }
        }

        onAccepted: {
            var url = ClientWrapper.utilsAdaptor.getAbsPath(file.toString())
            changeFilePrivateKey(url)
        }
    }

    id: advancedSIPSettingsViewLayout
    Layout.fillWidth: true
    spacing: 24

    property int preferredColumnWidth : sipAccountViewRect.width / 2 - 50
    property int preferredSettingsWidth: sipAccountViewRect.width - 80

    // call setting section
    ColumnLayout {

        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true

            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Call Settings")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredColumnWidth
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: checkBoxAutoAnswerSIP

                labelText: autoAnswerCallsText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setAutoAnswerCalls(checked)
                }
            }

            TextMetrics {
                id: autoAnswerCallsText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Auto Answer Calls")
            }

            ToggleSwitch {
                id: checkBoxCustomRingtoneSIP

                labelText: enableCustomRingtoneSIPElidedText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setEnableRingtone(checked)
                    btnRingtoneSIP.enabled = checked
                }
            }

            TextMetrics {
                id: enableCustomRingtoneSIPElidedText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Enable Custom Ringtone")
            }


            RowLayout {
                Layout.fillWidth: true

                ElidedTextLabel {
                    Layout.fillWidth: true

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Select Custom Ringtone")
                    maxWidth: preferredColumnWidth
                    fontSize: JamiTheme.settingsFontSize
                }

                MaterialButton {
                    id: btnRingtoneSIP

                    Layout.minimumWidth: preferredColumnWidth
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.maximumWidth: preferredColumnWidth

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

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
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true

            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Voicemail")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredColumnWidth
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: JamiTheme.preferredFieldHeight
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Voicemail Dial Code")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            MaterialLineEdit {
                id: lineEditVoiceMailDialCode

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
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
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true

            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Security")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredColumnWidth
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: encryptMediaStreamsToggle

                labelText: encryptMediaStreamsText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseSRTP(checked)
                    enableSDESToggle.enabled = checked
                    fallbackRTPToggle.enabled = checked
                }
            }

            TextMetrics {
                id: encryptMediaStreamsText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Encrypt Media Streams (SRTP)")
            }

            ToggleSwitch {
                id: enableSDESToggle

                labelText: enableSDESText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseSDES(checked)
                }
            }

            TextMetrics {
                id: enableSDESText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Enable SDES(Key Exchange)")
            }

            ToggleSwitch {
                id: fallbackRTPToggle

                labelText: fallbackRTPText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseRTPFallback(checked)
                }
            }

            TextMetrics {
                id: fallbackRTPText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Can Fallback on RTP")
            }

            ToggleSwitch {
                id: encryptNegotitationToggle

                labelText: encryptNegotitationText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseTLS(checked)
                    btnSIPCACert.enabled = checked
                    btnSIPUserCert.enabled = checked
                    btnSIPPrivateKey.enabled = checked
                    lineEditSIPCertPassword.enabled = checked
                }
            }

            TextMetrics {
                id: encryptNegotitationText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Encrypt Negotiation (TLS)")
            }

            GridLayout {
                Layout.fillWidth: true

                rowSpacing: 8
                columnSpacing: 8

                rows: 4
                columns: 2

                ElidedTextLabel {
                    Layout.fillWidth: true

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("CA Certificate")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialButton {
                    id: btnSIPCACert

                    Layout.minimumWidth: preferredColumnWidth
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.maximumWidth: preferredColumnWidth
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("User Certificate")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialButton {
                    id: btnSIPUserCert

                    Layout.minimumWidth: preferredColumnWidth
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.maximumWidth: preferredColumnWidth

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Private Key")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialButton {
                    id: btnSIPPrivateKey

                    Layout.minimumWidth: preferredColumnWidth
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.maximumWidth: preferredColumnWidth

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

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

                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Private Key Password")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }


                MaterialLineEdit {
                    id: lineEditSIPCertPassword

                    fieldLayoutWidth: preferredColumnWidth
                    fieldLayoutHeight: JamiTheme.preferredFieldHeight
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

                labelText: verifyIncomingCertificatesServerText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setVerifyCertificatesServer(checked)
                }
            }

            TextMetrics {
                id: verifyIncomingCertificatesServerText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Verify Certificates (Server Side)")
            }

            ToggleSwitch {
                id: verifyIncomingCertificatesClientToogle

                labelText: verifyIncomingCertificatesClientText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setVerifyCertificatesClient(checked)
                }
            }

            TextMetrics {
                id: verifyIncomingCertificatesClientText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Verify Certificates (Client Side)")
            }

            ToggleSwitch {
                id: requireCeritificateForTLSIncomingToggle

                labelText: requireCeritificateForTLSIncomingText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setRequireCertificatesIncomingTLS(checked)
                }
            }

            TextMetrics {
                id: requireCeritificateForTLSIncomingText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("TLS Connections Require Certificate")
            }


            GridLayout {
                Layout.fillWidth: true

                rowSpacing: 8
                columnSpacing: 8

                rows: 3
                columns: 2

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("TLS Protocol Method")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                SettingParaCombobox {
                    id: tlsProtocolComboBox

                    Layout.minimumWidth: preferredColumnWidth
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.maximumWidth: preferredColumnWidth
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
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

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Outgoing TLS Server Name")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialLineEdit {
                    id: outgoingTLSServerNameLineEdit

                    fieldLayoutWidth: preferredColumnWidth
                    fieldLayoutHeight: JamiTheme.preferredFieldHeight
                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.outgoingTLSServerNameLineEditTextChanged(text)
                    }
                }

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("Negotiation Timeout (seconds)")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                SpinBox {
                    id: negotiationTimeoutSpinBox

                    Layout.maximumWidth: preferredColumnWidth
                    Layout.minimumWidth: preferredColumnWidth
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
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
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true

            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Connectivity")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredSettingsWidth
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            rowSpacing: 8
            columnSpacing: 8

            rows: 9
            columns: 2

            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Registration Expire Timeout (seconds)")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }


            SpinBox {
                id: registrationExpireTimeoutSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Newtwork interface")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            SpinBox {
                id: networkInterfaceSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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

                labelText: qsTr("Use UPnP")
                fontPointSize: JamiTheme.settingsFontSize

                Layout.columnSpan: 2

                onSwitchToggled: {
                   SettingsAdapter.setUseUPnP(checked)
                }
            }

            // 4th row
            ToggleSwitch {
                id: checkBoxTurnEnableSIP

                labelText: qsTr("Use TURN")
                fontPointSize: JamiTheme.settingsFontSize

                Layout.columnSpan: 2

                onSwitchToggled: {
                   SettingsAdapter.setUseTURN(checked)
                    lineEditTurnAddressSIP.enabled = checked
                    lineEditTurnUsernameSIP.enabled = checked
                    lineEditTurnPsswdSIP.enabled = checked
                    lineEditTurnRealmSIP.enabled = checked
                }
            }

            // 5th row
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                text: qsTr("TURN Address")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            MaterialLineEdit {
                id: lineEditTurnAddressSIP

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("TURN Username")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            MaterialLineEdit {
                id: lineEditTurnUsernameSIP

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.setTURNUsername(text)
                }
            }

            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("TURN Password")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            MaterialLineEdit {
                id: lineEditTurnPsswdSIP

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("TURN Realm")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            MaterialLineEdit {
                id: lineEditTurnRealmSIP

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("STUN Address")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            MaterialLineEdit {
                id: lineEditSTUNAddressSIP

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
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
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            text: qsTr("Public Address")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredSettingsWidth
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            rowSpacing: 8
            columnSpacing: 8

            rows: 3
            columns: 2

            // 1st row
            ToggleSwitch {
                id: checkBoxCustomAddressPort

                labelText: checkBoxCustomAddressPortText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                Layout.columnSpan: 2

                onSwitchToggled: {
                   SettingsAdapter.setUseCustomAddressAndPort(checked)
                    lineEditSIPCustomAddress.enabled = checked
                    customPortSIPSpinBox.enabled = checked
                }
            }

            TextMetrics {
                id: checkBoxCustomAddressPortText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Use Custom Address/Port")
            }

            //2nd row
            ElidedTextLabel {
                Layout.leftMargin: JamiTheme.preferredMarginSize

                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Address")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            MaterialLineEdit {
                id: lineEditSIPCustomAddress

                fieldLayoutWidth: preferredColumnWidth
                fieldLayoutHeight: JamiTheme.preferredFieldHeight
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
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Port")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            SpinBox {
                id: customPortSIPSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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
        spacing: 8
        Layout.fillWidth: true

        Label {
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            text: qsTr("Media")

            font.pointSize: JamiTheme.headerFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        ColumnLayout {
            spacing: 8
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: videoCheckBoxSIP

                labelText: videoCheckBoxSIPText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setVideoState(checked)
                }
            }

            TextMetrics {
                id: videoCheckBoxSIPText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Enable Video")
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

                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            maxWidth: preferredColumnWidth - 50
                            eText:  qsTr("Video Codecs")
                            fontSize: JamiTheme.settingsFontSize
                        }


                        HoverableRadiusButton {
                            id: videoDownPushButtonSIP

                            Layout.minimumWidth: 24
                            Layout.preferredWidth: 24
                            Layout.maximumWidth: 24

                            Layout.minimumHeight: 24
                            Layout.preferredHeight: 24
                            Layout.maximumHeight: 24

                            buttonImageHeight: height
                            buttonImageWidth: height
                            radius: height / 2

                            icon.source: "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                            icon.width: 24
                            icon.height: 24

                            onClicked: {
                                decreaseVideoCodecPriority()
                            }
                        }

                        HoverableRadiusButton {
                            id: videoUpPushButtonSIP

                            Layout.minimumWidth: 24
                            Layout.preferredWidth: 24
                            Layout.maximumWidth: 24

                            Layout.minimumHeight: 24
                            Layout.preferredHeight: 24
                            Layout.maximumHeight: 24

                            buttonImageHeight: height
                            buttonImageWidth: height
                            radius: height / 2

                            icon.source: "qrc:/images/icons/round-arrow_drop_up-24px.svg"
                            icon.width: 24
                            icon.height: 24

                            onClicked: {
                                increaseVideoCodecPriority()
                            }
                        }
                    }

                    ListViewJami {
                        id: videoListWidgetSIP

                        Layout.minimumWidth: preferredColumnWidth
                        Layout.preferredWidth: preferredColumnWidth
                        Layout.maximumWidth: preferredColumnWidth

                        Layout.minimumHeight: 192
                        Layout.preferredHeight: 192
                        Layout.maximumHeight: 192

                        model: videoCodecListModelSIP

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

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.maximumHeight: JamiTheme.preferredFieldHeight

                        ElidedTextLabel {
                            Layout.fillWidth: true

                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            maxWidth: preferredColumnWidth - 50
                            eText:  qsTr("Audio Codecs")
                            fontSize: JamiTheme.settingsFontSize
                        }


                        HoverableRadiusButton {
                            id: audioDownPushButtonSIP

                            Layout.minimumWidth: 24
                            Layout.preferredWidth: 24
                            Layout.maximumWidth: 24

                            Layout.minimumHeight: 24
                            Layout.preferredHeight: 24
                            Layout.maximumHeight: 24

                            radius: height / 2
                            buttonImageHeight: height
                            buttonImageWidth: height

                            icon.source: "qrc:/images/icons/round-arrow_drop_down-24px.svg"
                            icon.width: 24
                            icon.height: 24

                            onClicked: {
                                decreaseAudioCodecPriority()
                            }
                        }

                        HoverableRadiusButton {
                            id: audioUpPushButtonSIP

                            Layout.minimumWidth: 24
                            Layout.preferredWidth: 24
                            Layout.maximumWidth: 24

                            Layout.minimumHeight: 24
                            Layout.preferredHeight: 24
                            Layout.maximumHeight: 24

                            radius: height / 2
                            buttonImageHeight: height
                            buttonImageWidth: height

                            icon.source: "qrc:/images/icons/round-arrow_drop_up-24px.svg"
                            icon.width: 24
                            icon.height: 24

                            onClicked: {
                                increaseAudioCodecPriority()
                            }
                        }
                    }

                    ListViewJami {
                        id: audioListWidgetSIP

                        Layout.minimumWidth: preferredColumnWidth
                        Layout.preferredWidth: preferredColumnWidth
                        Layout.maximumWidth: preferredColumnWidth

                        Layout.minimumHeight: 192
                        Layout.preferredHeight: 192
                        Layout.maximumHeight: 192

                        model: audioCodecListModelSIP

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
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("SDP Session Negotiation (ICE Fallback)")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredSettingsWidth
        }

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight
            Layout.leftMargin: JamiTheme.preferredMarginSize

            eText: qsTr("Only used during negotiation in case ICE is not supported")
            fontSize: JamiTheme.settingsFontSize
            maxWidth: preferredSettingsWidth
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            rowSpacing: 8
            columnSpacing: 8

            rows: 4
            columns: 2

            // 1st row
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Audio RTP Min Port")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            SpinBox {
                id:audioRTPMinPortSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Audio RTP Max Port")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            SpinBox {
                id:audioRTPMaxPortSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Video RTP Min Port")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            SpinBox {
                id:videoRTPMinPortSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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
            ElidedTextLabel {
                Layout.fillWidth: true
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                eText: qsTr("Video RTP Max Port")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            SpinBox {
                id:videoRTPMaxPortSpinBox

                Layout.maximumWidth: preferredColumnWidth
                Layout.minimumWidth: preferredColumnWidth
                Layout.preferredWidth: preferredColumnWidth
                Layout.maximumHeight: JamiTheme.preferredFieldHeight
                Layout.minimumHeight: JamiTheme.preferredFieldHeight
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


