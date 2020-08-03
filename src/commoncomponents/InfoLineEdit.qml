import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Styles 1.4

import "../constant"

TextField{
    enum BorderColorMode{
        NORMAL,
        RIGHT,
        ERROR
    }

    property int fieldLayoutWidth: 256
    property int fieldLayoutHeight: 30
    property bool layoutFillwidth: false

    property int borderColorMode: InfoLineEdit.NORMAL
    property var backgroundColor: JamiTheme.rgb256(240,240,240)
    property var borderColor: {
        switch(borderColorMode){
        case InfoLineEdit.NORMAL:
            return "transparent"
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
    font.kerning: true
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter

    background: Rectangle {
        anchors.fill: parent
        radius: readOnly? 0 : height / 2
        border.color: readOnly? "transparent" : borderColor
        border.width:readOnly? 0 : 2
        color: readOnly? "transparent" : backgroundColor
    }
}
