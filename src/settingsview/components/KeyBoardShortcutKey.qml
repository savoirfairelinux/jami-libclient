import QtQuick 2.9
import QtQuick.Controls 2.2

Item {
    property alias text: shortcutText.text
    Rectangle{
        id: keyRect
        width: t_metrics.tightBoundingRect.width + 10
        height: t_metrics.tightBoundingRect.height + 10
        color: "#525252"
        radius: 5
        anchors.centerIn: parent
        Text {
            id : shortcutText
            anchors.centerIn: parent
            anchors.leftMargin: 10
            font.family: "Arial"
            font.pointSize: 12
            color: "white"
        }
        TextMetrics {
            id:     t_metrics
            font:   shortcutText.font
            text:   shortcutText.text
        }
    }
}
