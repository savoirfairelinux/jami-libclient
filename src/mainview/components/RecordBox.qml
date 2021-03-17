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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Shapes 1.14

import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Constants 1.0

import "../../commoncomponents"


Rectangle {

    enum States {
        INIT,
        RECORDING,
        REC_SUCCESS
    }

    id: recBox
    width: 320
    height: 240
    radius: 5
    border.color: JamiTheme.tabbarBorderColor
    color: JamiTheme.backgroundColor

    property string pathRecorder: ""
    property string timeText: "00:00"
    property int duration: 0
    property int state: RecordBox.States.INIT
    property bool isVideo: false
    property bool previewAvailable: false
    property int preferredWidth: 320
    property int preferredHeight: 240
    property int btnSize: 40

    property int offset: 3
    property int curveRadius: 6
    property int x_offset: 0
    property int y_offset: 0

    function openRecorder(set_x, set_y, vid) {

        focus = true
        visible = true
        isVideo = vid

        x_offset = (isVideo ? -34 : -80)
        scaleHeight()
        y_offset = -64-height

        updateState(RecordBox.States.INIT)

        if (isVideo){
            AccountAdapter.startPreviewing(false)
            previewAvailable = true
        }
    }

    function scaleHeight(){
        height = preferredHeight
        if (isVideo) {
            var device = AVModel.getDefaultDevice()
            var settings = SettingsAdapter.get_Video_Settings_Size(device)
            var res = settings.split("x")
            var aspectRatio = res[1] / res[0]
            if (aspectRatio) {
                height = preferredWidth*aspectRatio
            } else {
                console.error("Could not scale recording video preview")
            }
        }
    }

    onActiveFocusChanged:  {
        if (visible) {
            closeRecorder()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            closeRecorder()
        }
    }

    function closeRecorder() {
        if (isVideo && AccountAdapter.isPreviewing()) {
            AccountAdapter.stopPreviewing()
        }
        stopRecording()
        visible = false
    }

    function updateState(new_state) {
        state = new_state
        recordButton.visible = (state == RecordBox.States.INIT)
        btnStop.visible = (state == RecordBox.States.RECORDING)
        btnRestart.visible = (state == RecordBox.States.REC_SUCCESS)
        btnSend.visible = (state == RecordBox.States.REC_SUCCESS)

        if (state == RecordBox.States.INIT) {
            duration = 0
            time.text = "00:00"
            timer.stop()
        } else if (state == RecordBox.States.REC_SUCCESS) {
            timer.stop()
        }
    }

    function startRecording() {
        timer.start()
        pathRecorder = AVModel.startLocalRecorder(!isVideo)
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

        time.text = min + ":" + sec;
    }

    Shape {
        id: backgroundShape
        width: recBox.width
        height: recBox.height
        anchors.centerIn: parent
        x: -offset
        y: -offset
        ShapePath {
            fillColor: JamiTheme.backgroundColor

            strokeWidth: 1
            strokeColor: JamiTheme.tabbarBorderColor

            startX: -offset+curveRadius; startY: -offset
            joinStyle: ShapePath.RoundJoin

            PathLine { x: width+offset-curveRadius; y: -offset }

            PathArc {
                x: width+offset; y: -offset+curveRadius
                radiusX: curveRadius; radiusY: curveRadius
            }

            PathLine { x: width+offset; y: height+offset-curveRadius }

            PathArc {
                x: width+offset-curveRadius; y: height+offset
                radiusX: curveRadius; radiusY: curveRadius
            }

            PathLine { x: width/2+10; y: height+offset }
            PathLine { x: width/2; y: height+offset+10 }
            PathLine { x: width/2-10; y: height+offset }


            PathLine { x: -offset+curveRadius; y: height+offset }

            PathArc {
                x: -offset; y: height+offset-curveRadius
                radiusX: curveRadius; radiusY: curveRadius
            }

            PathLine { x: -offset; y: -offset+curveRadius }

            PathArc {
                x: -offset+curveRadius; y: -offset
                radiusX: curveRadius; radiusY: curveRadius
            }
        }
    }

    Rectangle {
        id: rectBox
        visible: (isVideo && previewAvailable)
        Layout.alignment: Qt.AlignHCenter
        anchors.fill: parent
        color: JamiTheme.blackColor
        radius: 5

        PreviewRenderer{
            id: previewWidget

            anchors.fill: rectBox
            anchors.centerIn: rectBox

            lrcInstance: LRCInstance

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: rectBox
            }
        }
    }

    Label {
        visible: (isVideo && !previewAvailable)

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter

        text: qsTr("Preview unavailable")
        font.pointSize: 10
        font.kerning: true
    }

    Timer {
        id: timer
        interval: 1000;
        running: false;
        repeat: true
        onTriggered: updateTimer()
    }

    Text {
        id: time
        visible: true
        text: "00:00"
        color: (isVideo? JamiTheme.whiteColor : JamiTheme.textColor)
        font.pointSize: (isVideo ? 12 : 20)

        anchors.centerIn: recordButton
        anchors.horizontalCenterOffset: (isVideo ? 100 : 0)
        anchors.verticalCenterOffset: (isVideo ? 0 : -100)
    }

    PushButton {
        id: recordButton

        preferredSize: btnSize

        anchors.horizontalCenter: recBox.horizontalCenter
        anchors.bottom: recBox.bottom
        anchors.bottomMargin: 5

        normalColor: isVideo? "transparent" : JamiTheme.backgroundColor

        source: "qrc:/images/icons/av_icons/fiber_manual_record-24px.svg"
        imageColor: JamiTheme.recordIconColor

        onClicked: {
            updateState(RecordBox.States.RECORDING)
            startRecording()
        }
    }

    PushButton {
        id: btnStop

        preferredSize: btnSize

        anchors.horizontalCenter: recBox.horizontalCenter
        anchors.bottom: recBox.bottom
        anchors.bottomMargin: 5

        normalColor: isVideo? "transparent" : JamiTheme.backgroundColor

        source: "qrc:/images/icons/av_icons/stop-24px-red.svg"
        imageColor: isVideo? JamiTheme.whiteColor : JamiTheme.textColor

        onClicked: {
            stopRecording()
            updateState(RecordBox.States.REC_SUCCESS)
        }
    }

    PushButton {
        id: btnRestart

        preferredSize: btnSize

        anchors.horizontalCenter: recBox.horizontalCenter
        anchors.horizontalCenterOffset: -25
        anchors.bottom: recBox.bottom
        anchors.bottomMargin: 5

        normalColor: isVideo? "transparent" : JamiTheme.backgroundColor

        source: "qrc:/images/icons/av_icons/re-record-24px.svg"
        imageColor: isVideo? JamiTheme.whiteColor : JamiTheme.textColor

        onClicked: {
            stopRecording()
            updateState(RecordBox.States.INIT)
        }
    }

    PushButton {
        id: btnSend

        preferredSize: btnSize

        anchors.horizontalCenter: recBox.horizontalCenter
        anchors.horizontalCenterOffset: 25
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5

        normalColor: isVideo? "transparent" : JamiTheme.backgroundColor

        source: "qrc:/images/icons/av_icons/send-24px.svg"
        imageColor: isVideo? JamiTheme.whiteColor : JamiTheme.textColor

        onClicked: {
            stopRecording()
            sendRecord()
            closeRecorder()
            updateState(RecordBox.States.INIT)
        }
    }
}
