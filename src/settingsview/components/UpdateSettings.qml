/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import net.jami.Adapters 1.0
import net.jami.Enums 1.0
import net.jami.Models 1.0
import "../../commoncomponents"

ColumnLayout {
    id: root

    Label {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        text: JamiStrings.updatesTitle
        font.pointSize: JamiTheme.headerFontSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    ToggleSwitch {
        id: autoUpdateCheckBox

        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: SettingsAdapter.getAppValue(Settings.Key.AutoUpdate)

        labelText: JamiStrings.update
        tooltipText: JamiStrings.enableAutoUpdates
        fontPointSize: JamiTheme.settingsFontSize

        onSwitchToggled: {
            SettingsAdapter.setAppValue(Settings.Key.AutoUpdate, checked)
            UpdateManager.setAutoUpdateCheck(checked)
        }
    }

    MaterialButton {
        id: checkUpdateButton

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: JamiTheme.preferredFieldWidth
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
        hoveredColor: JamiTheme.buttonTintedBlackHovered
        pressedColor: JamiTheme.buttonTintedBlackPressed
        outlined: true

        toolTipText: JamiStrings.checkForUpdates
        text: JamiStrings.checkForUpdates

        onClicked: UpdateManager.checkForUpdates()
    }

    MaterialButton {
        id: installBetaButton

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: JamiTheme.preferredFieldWidth
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        color: enabled? JamiTheme.buttonTintedBlack : JamiTheme.buttonTintedGrey
        hoveredColor: JamiTheme.buttonTintedBlackHovered
        pressedColor: JamiTheme.buttonTintedBlackPressed
        outlined: true

        toolTipText: JamiStrings.betaInstall
        text: JamiStrings.betaInstall

        onClicked: {
            confirmInstallDialog.beta = true
            confirmInstallDialog.openWithParameters(JamiStrings.updateDialogTitle,
                                                    JamiStrings.confirmBeta)
        }
    }

    Component.onCompleted: {
        // Quiet check for updates on start if set to.
        if (SettingsAdapter.getAppValue(Settings.AutoUpdate)) {
            UpdateManager.checkForUpdates(true)
            UpdateManager.setAutoUpdateCheck(true)
        }
    }

    Connections {
        target: UpdateManager

        function errorToString(error) {
            switch(error){
            case NetWorkManager.ACCESS_DENIED:
                return JamiStrings.genericError
            case NetWorkManager.DISCONNECTED:
                return JamiStrings.networkDisconnected
            case NetWorkManager.NETWORK_ERROR:
            case NetWorkManager.SSL_ERROR:
                return JamiStrings.updateDownloadNetworkError
            case NetWorkManager.CANCELED:
                return JamiStrings.updateDownloadCanceled
            default: return {}
            }
        }

        function onUpdateCheckReplyReceived(ok, found) {
            if (!ok) {
                issueDialog.openWithParameters(JamiStrings.updateDialogTitle,
                                               JamiStrings.updateCheckError)
                return
            }
            if (!found) {
                issueDialog.openWithParameters(JamiStrings.updateDialogTitle,
                                               JamiStrings.updateNotFound)
            } else {
                confirmInstallDialog.openWithParameters(JamiStrings.updateDialogTitle,
                                                        JamiStrings.updateFound)
            }
        }

        function onUpdateCheckErrorOccurred(error) {
            issueDialog.openWithParameters(JamiStrings.updateDialogTitle,
                                           errorToString(error))
        }

        function onUpdateDownloadStarted() {
            downloadDialog.setDownloadProgress(0, 0)
            downloadDialog.openWithParameters(JamiStrings.updateDialogTitle)
        }

        function onUpdateDownloadProgressChanged(bytesRead, totalBytes) {
            downloadDialog.setDownloadProgress(bytesRead, totalBytes)
        }

        function onUpdateDownloadErrorOccurred(error) {
            downloadDialog.close()
            issueDialog.openWithParameters(JamiStrings.updateDialogTitle,
                                           errorToString(error))
        }

        function onUpdateDownloadFinished() { downloadDialog.close() }
    }

    SimpleMessageDialog {
        id: confirmInstallDialog

        property bool beta: false

        buttonTitles: [JamiStrings.optionOk, JamiStrings.optionCancel]
        buttonStyles: [
            SimpleMessageDialog.ButtonStyle.TintedBlue,
            SimpleMessageDialog.ButtonStyle.TintedBlue
        ]
        buttonCallBacks: [function() {UpdateManager.applyUpdates(beta)}]
    }

    SimpleMessageDialog {
        id: issueDialog

        buttonTitles: [JamiStrings.optionOk]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue]
        buttonCallBacks: []
    }

    SimpleMessageDialog {
        id: downloadDialog

        property int bytesRead: 0
        property int totalBytes: 0
        property string hSizeRead:  UtilsAdapter.humanFileSize(bytesRead)
        property string hTotalBytes: UtilsAdapter.humanFileSize(totalBytes)
        property alias progressBarValue: progressBar.value

        function setDownloadProgress(bytesRead, totalBytes) {
            downloadDialog.bytesRead = bytesRead
            downloadDialog.totalBytes = totalBytes
        }

        infoText: JamiStrings.updateDownloading +
                  " (%1 / %2)".arg(hSizeRead).arg(hTotalBytes)

        innerContentData: ProgressBar {
            id: progressBar

            value: downloadDialog.bytesRead /
                   downloadDialog.totalBytes

            anchors.left: parent.left
            anchors.leftMargin: JamiTheme.preferredMarginSize
            anchors.right: parent.right
            anchors.rightMargin: JamiTheme.preferredMarginSize

            background: Rectangle {
                implicitWidth: parent.width
                implicitHeight: 24
                color: JamiTheme.darkGrey
            }

            contentItem: Item {
                implicitWidth: parent.width
                implicitHeight: 22

                Rectangle {
                    width: progressBar.visualPosition * parent.width
                    height: parent.height
                    color: JamiTheme.selectionBlue
                }
                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    color: JamiTheme.white
                    font.bold: true
                    font.pointSize: JamiTheme.textFontSize + 1
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: Math.ceil(progressBar.value * 100).toString() + "%"
                }
            }
        }

        buttonTitles: [JamiStrings.optionCancel]
        buttonStyles: [SimpleMessageDialog.ButtonStyle.TintedBlue]
        buttonCallBacks: [function() {UpdateManager.cancelUpdate()}]
    }
}
