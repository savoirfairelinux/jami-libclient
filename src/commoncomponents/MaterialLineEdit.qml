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
        switch(borderColorMode){
        case InfoLineEdit.RIGHT:
        case InfoLineEdit.NORMAL:
            return ""
        case InfoLineEdit.ERROR:
            return "qrc:/images/icons/round-error-24px.svg"
        }
    }
    property var backgroundColor: JamiTheme.rgb256(240,240,240)
    property var borderColor: {
        switch(borderColorMode){
        case InfoLineEdit.NORMAL:
            return "black"
        case InfoLineEdit.RIGHT:
            return "green"
        case InfoLineEdit.ERROR:
            return "red"
        }
    }

    Layout.minimumHeight: fieldLayoutHeight
    Layout.preferredHeight: fieldLayoutHeight
    Layout.maximumHeight: fieldLayoutHeight

    Layout.minimumWidth: fieldLayoutWidth
    Layout.maximumWidth: fieldLayoutWidth
    Layout.preferredWidth: fieldLayoutWidth

    Layout.fillWidth: layoutFillwidth
    Layout.alignment: Qt.AlignHCenter

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
    }

    background: Rectangle {
        anchors.fill: parent
        radius: 4
        border.color: readOnly? "black" : borderColor
        color: readOnly? "transparent" : backgroundColor
    }
}
