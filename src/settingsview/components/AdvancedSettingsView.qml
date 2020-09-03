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
    function updateAccountInfoDisplayedAdvance() {
        //Call Settings
        checkAutoConnectOnLocalNetwork.checked = SettingsAdapter.getAccountConfig_PeerDiscovery()
        checkBoxUntrusted.checked = SettingsAdapter.getAccountConfig_DHT_PublicInCalls()
        checkBoxRdv.checked = SettingsAdapter.getAccountConfig_RendezVous()
        checkBoxAutoAnswer.checked = SettingsAdapter.getAccountConfig_AutoAnswer()
        checkBoxCustomRingtone.checked = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()

        // Name Server
        lineEditNameServer.text = SettingsAdapter.getAccountConfig_RingNS_Uri()

        //OpenDHT Config
        checkBoxEnableProxy.checked = SettingsAdapter.getAccountConfig_ProxyEnabled()
        lineEditProxy.text = SettingsAdapter.getAccountConfig_ProxyServer()
        lineEditBootstrap.text = SettingsAdapter.getAccountConfig_Hostname()

        // Security
        btnCACert.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_TLS_CertificateListFile())
        btnUserCert.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_TLS_CertificateFile())
        btnPrivateKey.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile())

        // Connectivity
        checkBoxUPnP.checked = SettingsAdapter.getAccountConfig_UpnpEnabled()
        checkBoxTurnEnable.checked = SettingsAdapter.getAccountConfig_TURN_Enabled()
        lineEditTurnAddress.text = SettingsAdapter.getAccountConfig_TURN_Server()
        lineEditTurnUsername.text = SettingsAdapter.getAccountConfig_TURN_Username()
        lineEditTurnPassword.text = SettingsAdapter.getAccountConfig_TURN_Password()
        checkBoxSTUNEnable.checked = SettingsAdapter.getAccountConfig_STUN_Enabled()
        lineEditSTUNAddress.text = SettingsAdapter.getAccountConfig_STUN_Server()
        // codecs
        videoCheckBox.checked = SettingsAdapter.getAccountConfig_Video_Enabled()
            // update audio and video codec, make sure this change does not trigger item change events
        updateAudioCodecs();
        updateVideoCodecs();
        btnRingtone.enabled = SettingsAdapter.getAccountConfig_Ringtone_RingtoneEnabled()
        btnRingtone.text = ClientWrapper.utilsAdaptor.toFileInfoName(ClientWrapper.SettingsAdapter.getAccountConfig_Ringtone_RingtonePath())
        lineEditProxy.enabled = SettingsAdapter.getAccountConfig_ProxyEnabled()
        lineEditSTUNAddress.enabled = SettingsAdapter.getAccountConfig_STUN_Enabled()
    }

    function updateAudioCodecs(){
        audioCodecListModel.layoutAboutToBeChanged()
        audioCodecListModel.dataChanged(audioCodecListModel.index(0, 0),
                                     audioCodecListModel.index(audioCodecListModel.rowCount() - 1, 0))
        audioCodecListModel.layoutChanged()
    }

    function updateVideoCodecs(){
        videoCodecListModel.layoutAboutToBeChanged()
        videoCodecListModel.dataChanged(videoCodecListModel.index(0, 0),
                                     videoCodecListModel.index(videoCodecListModel.rowCount() - 1, 0))
        videoCodecListModel.layoutChanged()
    }

    function decreaseAudioCodecPriority(){
        var index = audioListWidget.currentIndex
        var codecId = audioCodecListModel.data(audioCodecListModel.index(index,0), AudioCodecListModel.AudioCodecID)

       SettingsAdapter.decreaseAudioCodecPriority(codecId)
        audioListWidget.currentIndex = index + 1
        updateAudioCodecs()
    }

    function increaseAudioCodecPriority(){
        var index = audioListWidget.currentIndex
        var codecId = audioCodecListModel.data(audioCodecListModel.index(index,0), AudioCodecListModel.AudioCodecID)

       SettingsAdapter.increaseAudioCodecPriority(codecId)
        audioListWidget.currentIndex = index - 1
        updateAudioCodecs()
    }

    function decreaseVideoCodecPriority(){
        var index = videoListWidget.currentIndex
        var codecId = videoCodecListModel.data(videoCodecListModel.index(index,0), VideoCodecListModel.VideoCodecID)

       SettingsAdapter.decreaseVideoCodecPriority(codecId)
        videoListWidget.currentIndex = index + 1
        updateVideoCodecs()
    }

    function increaseVideoCodecPriority(){
        var index = videoListWidget.currentIndex
        var codecId = videoCodecListModel.data(videoCodecListModel.index(index,0), VideoCodecListModel.VideoCodecID)

       SettingsAdapter.increaseVideoCodecPriority(codecId)
        videoListWidget.currentIndex = index - 1
        updateVideoCodecs()
    }

    VideoCodecListModel{
        id: videoCodecListModel
    }

    AudioCodecListModel{
        id: audioCodecListModel
    }

    function changeRingtonePath(url){
        if(url.length !== 0) {
           SettingsAdapter.set_RingtonePath(url)
            btnRingtone.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        } else if (ClientWrapper.SettingsAdapter.getAccountConfig_Ringtone_RingtonePath().length === 0){
            btnRingtone.text = qsTr("Add a custom ringtone")
        }
    }

    function changeFileCACert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileCACert(url)
            btnCACert.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        }
    }

    function changeFileUserCert(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FileUserCert(url)
            btnUserCert.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        }
    }

    function changeFilePrivateKey(url){
        if(url.length !== 0) {
           SettingsAdapter.set_FilePrivateKey(url)
            btnPrivateKey.text = ClientWrapper.utilsAdaptor.toFileInfoName(url)
        }
    }

    JamiFileDialog {
        id: ringtonePath_Dialog

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
        id: caCert_Dialog

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
        id: userCert_Dialog

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
        id: privateKey_Dialog

        property string oldPath : {
            return ClientWrapper.SettingsAdapter.getAccountConfig_TLS_PrivateKeyFile()
        }
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

    id: advancedSettingsViewLayout
    Layout.fillWidth: true

    property int preferredColumnWidth : accountViewRect.width / 2 - 50

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
                id: checkBoxUntrusted

                labelText: allowIncomingCallsText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setCallsUntrusted(checked)
                }
            }

            TextMetrics {
                id: allowIncomingCallsText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Allow incoming calls from unknown contacts")
            }

            ToggleSwitch {
                id: checkBoxAutoAnswer

                Layout.fillWidth: true

                labelText: autoAnswerCallsElidedText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setAutoAnswerCalls(checked)
                }
            }

            TextMetrics {
                id: autoAnswerCallsElidedText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Auto Answer Calls")
            }

            ToggleSwitch {
                id: checkBoxCustomRingtone

                labelText: enableCustomRingtoneElidedText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setEnableRingtone(checked)
                    btnRingtone.enabled = checked
                }
            }

            TextMetrics {
                id: enableCustomRingtoneElidedText
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
                    id: btnRingtone

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: preferredColumnWidth
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        ringtonePath_Dialog.open()
                    }
                }
            }

            ToggleSwitch {
                id: checkBoxRdv

                labelText: qsTr("(Experimental) Rendez-vous: turn your account into a conference room")
                fontPointSize: 10

                onSwitchToggled: {
                   SettingsAdapter.setIsRendezVous(checked)
                }
            }
        }
    }

    ColumnLayout {
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Name Server")
            maxWidth: preferredColumnWidth
            fontSize: JamiTheme.headerFontSize
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

                eText: qsTr("Address")
                fontSize: JamiTheme.settingsFontSize
                maxWidth: preferredColumnWidth
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            MaterialLineEdit {
                id: lineEditNameServer

                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: JamiTheme.preferredFieldHeight
                Layout.preferredWidth: preferredColumnWidth

                padding: 8

                font.pointSize: JamiTheme.settingsFontSize
                font.kerning: true

                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                   SettingsAdapter.setNameServer(text)
                }
            }
        }
    }


    ColumnLayout {
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("OpenDHT Configuration")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredColumnWidth
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: checkBoxEnableProxy

                labelText: qsTr("Enable proxy")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setEnableProxy(checked)
                    lineEditProxy.enabled = checked
                }
            }

            RowLayout {
                Layout.fillWidth: true

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    text: qsTr("Proxy Address")
                    font.pointSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialLineEdit {
                    id: lineEditProxy

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.setProxyAddress(text)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    text: qsTr("Bootstrap")
                    font.pointSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                MaterialLineEdit {
                    id: lineEditBootstrap

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.setBootstrapAddress(text)
                    }
                }
            }
        }
    }

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

            GridLayout {
                rows: 4
                columns: 2
                rowSpacing: 8
                columnSpacing: 8

                Layout.fillWidth: true

                // CA Certificate
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
                    id: btnCACert

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        caCert_Dialog.open()
                    }
                }

                // User Certificate
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
                    id: btnUserCert

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        userCert_Dialog.open()
                    }
                }

                // Private Key
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
                    id: btnPrivateKey

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    source: "qrc:/images/icons/round-folder-24px.svg"
                    color: JamiTheme.buttonTintedGrey
                    hoveredColor: JamiTheme.buttonTintedGreyHovered
                    pressedColor: JamiTheme.buttonTintedGreyPressed

                    onClicked: {
                        privateKey_Dialog.open()
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
                    id: lineEditCertPassword

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    echoMode: TextInput.Password
                }
            }
        }
    }

    ColumnLayout {
        spacing: 8
        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true
            Layout.topMargin: JamiTheme.preferredMarginSize

            Layout.minimumHeight: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight
            Layout.maximumHeight: JamiTheme.preferredFieldHeight

            eText: qsTr("Connectivity")
            fontSize: JamiTheme.headerFontSize
            maxWidth: preferredColumnWidth
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: checkAutoConnectOnLocalNetwork

                Layout.fillWidth: true

                labelText: autoConnectOnLocalNetworkElidedText.elidedText
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setAutoConnectOnLocalNetwork(checked)
                }
            }

            TextMetrics {
                id: autoConnectOnLocalNetworkElidedText
                elide: Text.ElideRight
                elideWidth: preferredColumnWidth
                text: qsTr("Auto Connect On Local Network")
            }

            ToggleSwitch {
                id: checkBoxUPnP

                Layout.fillWidth: true

                labelText: qsTr("Use UPnP")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseUPnP(checked)
                }
            }

            ToggleSwitch {
                id: checkBoxTurnEnable

                Layout.fillWidth: true

                labelText: qsTr("Use TURN")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseTURN(checked)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("TURN Address")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialLineEdit {
                    id: lineEditTurnAddress

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.setTURNAddress(text)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("TURN Username")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialLineEdit {
                    id: lineEditTurnUsername

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.setTURNUsername(text)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                ElidedTextLabel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight
                    eText: qsTr("TURN Password")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialLineEdit {
                    id: lineEditTurnPassword

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8
                    layer.mipmap: false

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    echoMode: TextInput.Password

                    onEditingFinished: {
                       SettingsAdapter.setTURNPassword(text)
                    }
                }
            }


            ToggleSwitch {
                id: checkBoxSTUNEnable

                Layout.fillWidth: true

                labelText: qsTr("Use STUN")
                fontPointSize: JamiTheme.settingsFontSize

                onSwitchToggled: {
                   SettingsAdapter.setUseSTUN(checked)
                    lineEditSTUNAddress.enabled = checked
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: JamiTheme.preferredFieldHeight

                ElidedTextLabel {
                    id: lblEditSTUNAddress

                    Layout.fillWidth: true
                    Layout.minimumHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.maximumHeight: JamiTheme.preferredFieldHeight

                    eText: qsTr("STUN Address")
                    fontSize: JamiTheme.settingsFontSize
                    maxWidth: preferredColumnWidth
                }

                MaterialLineEdit {
                    id: lineEditSTUNAddress

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: JamiTheme.preferredFieldHeight
                    Layout.preferredWidth: preferredColumnWidth

                    padding: 8

                    font.pointSize: JamiTheme.settingsFontSize
                    font.kerning: true

                    placeholderText: qsTr("STUN Address")

                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter

                    onEditingFinished: {
                       SettingsAdapter.setSTUNAddress(text)
                    }
                }
            }
        }
    }

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
            Layout.fillWidth: true
            Layout.leftMargin: JamiTheme.preferredMarginSize

            ToggleSwitch {
                id: videoCheckBox

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
                            Layout.minimumHeight: JamiTheme.preferredFieldHeight
                            Layout.preferredHeight: JamiTheme.preferredFieldHeight
                            Layout.maximumHeight: JamiTheme.preferredFieldHeight

                            maxWidth: preferredColumnWidth - 50
                            eText:  qsTr("Video Codecs")
                            fontSize: JamiTheme.settingsFontSize
                        }

                        HoverableRadiusButton {
                            id: videoDownPushButton

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
                            id: videoUpPushButton

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
                        id: videoListWidget

                        Layout.minimumWidth: preferredColumnWidth
                        Layout.preferredWidth: preferredColumnWidth
                        Layout.maximumWidth: preferredColumnWidth
                        Layout.minimumHeight: 192
                        Layout.preferredHeight: 192
                        Layout.maximumHeight: 192

                        model: videoCodecListModel

                        delegate: VideoCodecDelegate {
                            id: videoCodecDelegate

                            width: videoListWidget.width
                            height: videoListWidget.height / 4

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
                            id: audioDownPushButton

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
                            id: audioUpPushButton

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
                        id: audioListWidget

                        Layout.minimumWidth: preferredColumnWidth
                        Layout.preferredWidth: preferredColumnWidth
                        Layout.maximumWidth: preferredColumnWidth
                        Layout.minimumHeight: 192
                        Layout.preferredHeight: 192
                        Layout.maximumHeight: 192

                        model: audioCodecListModel

                        delegate: AudioCodecDelegate {
                            id: audioCodecDelegate

                            width: audioListWidget.width
                            height: audioListWidget.height / 4

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
}
