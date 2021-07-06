import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtWebEngine 1.10

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

Control {
    id: root

    readonly property bool isGenerated: Type === Interaction.Type.CALL ||
                                        Type === Interaction.Type.CONTACT
    readonly property string author: Author
    readonly property var timestamp: Timestamp
    readonly property bool isOutgoing: model.Author === ""
    readonly property var formattedTime: MessagesAdapter.getFormattedTime(Timestamp)
    readonly property bool isImage: MessagesAdapter.isImage(Body)
    readonly property bool isAnimatedImage: MessagesAdapter.isAnimatedImage(Body)
    readonly property var linkPreviewInfo: LinkPreviewInfo

    readonly property var body: Body
    readonly property real msgMargin: 64

    width: parent ? parent.width : 0
    height: loader.height

    Loader {
        id: loader

        property alias isOutgoing: root.isOutgoing
        property alias isGenerated: root.isGenerated
        readonly property var author: Author
        readonly property var body: Body

        sourceComponent: isGenerated ?
                             generatedMsgComp :
                             userMsgComp
    }

    Component {
        id: generatedMsgComp

        Column {
            width: root.width
            spacing: 2

            TextArea {
                width: parent.width
                text: body
                horizontalAlignment: Qt.AlignHCenter
                readOnly: true
                font.pointSize: 11
                color: JamiTheme.chatviewTextColor
            }

            Item {
                id: infoCell

                width: parent.width
                height: childrenRect.height

                Component.onCompleted: children = timestampLabel
            }

            bottomPadding: 12
        }
    }

    Component {
        id: userMsgComp

        GridLayout {
            id: gridLayout

            width: root.width

            columns: 2
            rows: 2

            columnSpacing: 2
            rowSpacing: 2

            Column {
                id: msgCell

                Layout.column: isOutgoing ? 0 : 1
                Layout.row: 0
                Layout.fillWidth: true
                Layout.maximumWidth: 640
                Layout.preferredHeight: childrenRect.height
                Layout.alignment: isOutgoing ? Qt.AlignRight : Qt.AlignLeft
                Layout.leftMargin: isOutgoing ? msgMargin : 0
                Layout.rightMargin: isOutgoing ? 0 : msgMargin

                Control {
                    id: msgBlock

                    width: parent.width

                    contentItem: Column {
                        id: msgContent

                        property real txtWidth: ta.contentWidth + 3 * ta.padding

                        TextArea {
                            id: ta
                            width: parent.width
                            text: body
                            padding: 10
                            font.pointSize: 11
                            font.hintingPreference: Font.PreferNoHinting
                            renderType: Text.NativeRendering
                            textFormat: TextEdit.RichText
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            transform: Translate { x: bg.x }
                            rightPadding: isOutgoing ? padding * 1.5 : 0
                            color: isOutgoing ?
                                       JamiTheme.messageOutTxtColor :
                                       JamiTheme.messageInTxtColor
                        }
                    }
                    background: Rectangle {
                        id: bg

                        anchors.right: isOutgoing ? msgContent.right : undefined
                        width: msgContent.txtWidth
                        radius: 18
                        color: isOutgoing ?
                                   JamiTheme.messageOutBgColor :
                                   JamiTheme.messageInBgColor
                    }
                }
            }
            Item {
                id: infoCell

                Layout.column: isOutgoing ? 0 : 1
                Layout.row: 1
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height

                Component.onCompleted: children = timestampLabel
            }
            Item {
                id: avatarCell

                Layout.column: isOutgoing ? 1 : 0
                Layout.row: 0
                Layout.preferredWidth: isOutgoing ? 16 : avatar.width
                Layout.preferredHeight: msgCell.height
                Layout.leftMargin: isOutgoing ? 0 : 6
                Layout.rightMargin: Layout.leftMargin
                Avatar {
                    id: avatar
                    visible: !isOutgoing
                    anchors.bottom: parent.bottom
                    width: 32
                    height: 32
                    imageId: author
                    showPresenceIndicator: false
                    mode: Avatar.Mode.Contact
                }
            }
        }
    }

    Label {
        id: timestampLabel

        text: formattedTime
        color: JamiTheme.timestampColor

        anchors.right: isGenerated || !isOutgoing ? undefined : parent.right
        anchors.rightMargin: 6
        anchors.left: isGenerated || isOutgoing ? undefined : parent.left
        anchors.leftMargin: 6
        anchors.horizontalCenter: isGenerated ? parent.horizontalCenter : undefined
    }

    opacity: 0
    Behavior on opacity { NumberAnimation { duration: 40 } }

    Component.onCompleted: {
        opacity = 1
        if (!Linkified && !isImage && !isAnimatedImage) {
            MessagesAdapter.parseMessageUrls(Id, Body)
        }
    }
}
