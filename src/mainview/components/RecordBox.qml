/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Albert Babí <albert.babi@savoirfairelinux.com>
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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Shapes 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Rectangle {
    id: root

    enum States {
        INIT,
        RECORDING,
        REC_SUCCESS
    }

    property string pathRecorder: ""
    property string timeText: "00:00"
    property int duration: 0
    property int state: RecordBox.States.INIT
    property bool isVideo: false
    property int preferredWidth: 320
    property int preferredHeight: 240
    property int btnSize: 40

    property int offset: 3
    property int curveRadius: 6
    property int spikeHeight: 10 + offset

    function openRecorder(vid) {
        focus = true
        visible = true
        isVideo = vid

        scaleHeight()
        updateState(RecordBox.States.INIT)

        if (isVideo) {
            previewWidget.deviceId = VideoDevices.getDefaultDevice()
            previewWidget.rendererId = VideoDevices.startDevice(previewWidget.deviceId)
        }
    }

    function scaleHeight() {
        height = preferredHeight
        if (isVideo) {
            var resolution = VideoDevices.defaultRes
            var resVec = resolution.split("x")
            var aspectRatio = resVec[1] / resVec[0]
            if (aspectRatio) {
                height = preferredWidth * aspectRatio
            } else {
                console.error("Could not scale recording video preview")
            }
        }
    }

    function closeRecorder() {
        if (isVideo) {
            VideoDevices.stopDevice(previewWidget.deviceId)
        }
        stopRecording()
        visible = false
    }

    function updateState(new_state) {
        state = new_state
        recordButton.visible = (state === RecordBox.States.INIT)
        btnStop.visible = (state === RecordBox.States.RECORDING)
        btnRestart.visible = (state === RecordBox.States.REC_SUCCESS)
        btnSend.visible = (state === RecordBox.States.REC_SUCCESS)

        if (state === RecordBox.States.INIT) {
            duration = 0
            time.text = "00:00"
            timer.stop()
        } else if (state === RecordBox.States.REC_SUCCESS) {
            timer.stop()
        }
    }

    function startRecording() {
        timer.start()
        pathRecorder = AVModel.startLocalMediaRecorder(isVideo? VideoDevices.getDefaultDevice() : "")
        if (pathRecorder == "") {
            timer.stop()
        }
    }

    function stopRecording() {
        if (pathRecorder !== "") {
            AVModel.stopLocalRecorder(pathRecorder)
        }
    }

    function sendRecord() {
        if (pathRecorder !== "") {
            MessagesAdapter.sendFile(pathRecorder)
        }
    }

    function updateTimer() {

        duration += 1

        var m = Math.trunc(duration / 60)
        var s = (duration % 60)

        var min = (m < 10) ? "0" + String(m) : String(m)
        var sec = (s < 10) ? "0" + String(s) : String(s)

        time.text = min + ":" + sec
    }

    width: 320
    height: 240
    radius: 5
    border.color: JamiTheme.tabbarBorderColor
    color: JamiTheme.backgroundColor

    onActiveFocusChanged: {
        if (visible) {
            closeRecorder()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            closeRecorder()
        }
    }

    Shape {
        id: backgroundShape

        anchors.centerIn: parent

        width: root.width
        height: root.height

        x: -offset
        y: -offset

        ShapePath {
            fillColor: JamiTheme.backgroundColor

            strokeWidth: 1
            strokeColor: JamiTheme.tabbarBorderColor

            startX: -offset + curveRadius
            startY: -offset
            joinStyle: ShapePath.RoundJoin

            PathLine {
                x: width + offset - curveRadius
                y: -offset
            }

            PathArc {
                x: width + offset
                y: -offset + curveRadius
                radiusX: curveRadius
                radiusY: curveRadius
            }

            PathLine {
                x: width + offset
                y: height + offset - curveRadius
            }

            PathArc {
                x: width + offset - curveRadius
                y: height + offset
                radiusX: curveRadius
                radiusY: curveRadius
            }

            PathLine {
                x: width / 2 + 10
                y: height + offset
            }
            PathLine {
                x: width / 2
                y: height + spikeHeight
            }
            PathLine {
                x: width / 2 - 10
                y: height + offset
            }

            PathLine {
                x: -offset + curveRadius
                y: height + offset
            }

            PathArc {
                x: -offset
                y: height + offset - curveRadius
                radiusX: curveRadius
                radiusY: curveRadius
            }

            PathLine {
                x: -offset
                y: -offset + curveRadius
            }

            PathArc {
                x: -offset + curveRadius
                y: -offset
                radiusX: curveRadius
                radiusY: curveRadius
            }
        }
    }

    Rectangle {
        id: rectBox

        anchors.fill: parent

        visible: (isVideo && VideoDevices.listSize !== 0)
        color: JamiTheme.blackColor
        radius: 5

        PreviewRenderer {
            id: previewWidget

            anchors.fill: rectBox
            anchors.centerIn: rectBox
            property string deviceId: VideoDevices.getDefaultDevice()
            rendererId: VideoDevices.getDefaultDevice()

            lrcInstance: LRCInstance

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: rectBox
            }
        }

        onVisibleChanged: {
            if (visible) {
                previewWidget.deviceId = VideoDevices.getDefaultDevice()
                previewWidget.rendererId = VideoDevices.startDevice(previewWidget.deviceId)
            } else
                VideoDevices.stopDevice(previewWidget.deviceId)
        }
    }

    Label {
        anchors.centerIn: parent

        width: root.width

        visible: (isVideo && VideoDevices.listSize === 0)

        onVisibleChanged: {
            if (visible) {
                closeRecorder()
            }
        }

        text: JamiStrings.previewUnavailable
        font.pointSize: JamiTheme.settingsFontSize
        font.kerning: true
        color: JamiTheme.primaryForegroundColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Timer {
        id: timer

        interval: 1000
        running: false
        repeat: true

        onTriggered: updateTimer()
    }

    Text {
        id: time

        anchors.centerIn: recordButton
        anchors.horizontalCenterOffset: (isVideo ? 100 : 0)
        anchors.verticalCenterOffset: (isVideo ? 0 : -100)

        visible: true
        text: "00:00"
        color: (isVideo ? JamiTheme.whiteColor : JamiTheme.textColor)
        font.pointSize: (isVideo ? 12 : 20)
    }

    PushButton {
        id: recordButton

        anchors.horizontalCenter: root.horizontalCenter
        anchors.bottom: root.bottom
        anchors.bottomMargin: 5

        preferredSize: btnSize

        normalColor: isVideo ? "transparent" : JamiTheme.backgroundColor

        source: JamiResources.fiber_manual_record_24dp_svg
        imageColor: JamiTheme.recordIconColor

        onClicked: {
            updateState(RecordBox.States.RECORDING)
            startRecording()
        }
    }

    PushButton {
        id: btnStop

        anchors.horizontalCenter: root.horizontalCenter
        anchors.bottom: root.bottom
        anchors.bottomMargin: 5

        preferredSize: btnSize

        normalColor: isVideo ? "transparent" : JamiTheme.backgroundColor

        source: JamiResources.stop_24dp_red_svg
        imageColor: isVideo ? JamiTheme.whiteColor : JamiTheme.textColor

        onClicked: {
            stopRecording()
            updateState(RecordBox.States.REC_SUCCESS)
        }
    }

    PushButton {
        id: btnRestart

        anchors.horizontalCenter: root.horizontalCenter
        anchors.horizontalCenterOffset: -25
        anchors.bottom: root.bottom
        anchors.bottomMargin: 5

        preferredSize: btnSize

        normalColor: isVideo ? "transparent" : JamiTheme.backgroundColor

        source: JamiResources.re_record_24dp_svg
        imageColor: isVideo ? JamiTheme.whiteColor : JamiTheme.textColor

        onClicked: {
            stopRecording()
            updateState(RecordBox.States.INIT)
        }
    }

    PushButton {
        id: btnSend

        anchors.horizontalCenter: root.horizontalCenter
        anchors.horizontalCenterOffset: 25
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5

        preferredSize: btnSize

        normalColor: isVideo ? "transparent" : JamiTheme.backgroundColor

        source: JamiResources.send_24dp_svg
        imageColor: isVideo ? JamiTheme.whiteColor : JamiTheme.textColor

        onClicked: {
            stopRecording()
            sendRecord()
            closeRecorder()
            updateState(RecordBox.States.INIT)
        }
    }
}
