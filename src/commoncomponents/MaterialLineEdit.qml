import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.15

import "../constant"

TextField {
    enum BorderColorMode {
        NORMAL,
        RIGHT,
        ERROR
    }

    property int fieldLayoutWidth: 256
    property int fieldLayoutHeight: 48
    property bool layoutFillwidth: false

    property int borderColorMode: InfoLineEdit.NORMAL
    property var iconSource: {
        if (readOnly) {
            return ""
        }
        switch(borderColorMode){
        case InfoLineEdit.RIGHT:
            return "qrc:/images/icons/round-check_circle-24px.svg"
        case InfoLineEdit.NORMAL:
            return ""
        case InfoLineEdit.ERROR:
            return "qrc:/images/icons/round-error-24px.svg"
        }
    }
    property var backgroundColor: JamiTheme.rgb256(240,240,240)
    property var borderColor: {
        if (!enabled) {
            return "transparent"
        }
        switch(borderColorMode){
        case InfoLineEdit.NORMAL:
            return "#333"
        case InfoLineEdit.RIGHT:
            return "green"
        case InfoLineEdit.ERROR:
            return "red"
        }
    }

    signal imageClicked

    wrapMode: Text.Wrap
    readOnly: false
    selectByMouse: true
    font.pointSize: 10
    padding: 16
    font.kerning: true
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter

    Image {
        source: iconSource
        width: 24
        height: 24
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 16
        layer {
            enabled: true
            effect: ColorOverlay {
                id: overlay
                color: borderColor
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            enabled: borderColorMode === InfoLineEdit.RIGHT

            onReleased: {
                imageClicked()
            }
        }
    }

    background: Rectangle {
        anchors.fill: parent
        radius: 4
        border.color: readOnly? "transparent" : borderColor
        color: readOnly? "transparent" : backgroundColor
    }
}
