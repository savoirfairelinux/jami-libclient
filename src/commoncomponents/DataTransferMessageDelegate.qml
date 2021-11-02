/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Trevor Tabah <trevor.tabah@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import QtWebEngine 1.10

import net.jami.Models 1.1
import net.jami.Constants 1.1
import net.jami.Adapters 1.1

Loader {
    id: root

    property var mediaInfo
    property bool showTime: false
    property int seq: MsgSeq.single
    property string author: Author

    width: ListView.view ? ListView.view.width : 0

    sourceComponent: {
        if (Status === Interaction.Status.TRANSFER_FINISHED) {
            mediaInfo = MessagesAdapter.getMediaInfo(Body)
            if (Object.keys(mediaInfo).length !== 0)
                return localMediaMsgComp
        }
        return dataTransferMsgComp
    }

    opacity: 0
    Behavior on opacity { NumberAnimation { duration: 100 } }
    onLoaded: opacity = 1

    Component {
        id: dataTransferMsgComp

        SBSMessageBase {
            id: dataTransferItem

            property var transferStats: MessagesAdapter.getTransferStats(Id, Status)
            property bool canOpen: Status === Interaction.Status.TRANSFER_FINISHED || isOutgoing
            property real maxMsgWidth: root.width - senderMargin -
                                       2 * hPadding - avatarBlockWidth
                                       - buttonsLoader.width - 24 - 6 - 24

            isOutgoing: Author === ""
            showTime: root.showTime
            seq: root.seq
            author: Author
            location: Body
            transferName: TransferName
            transferId: Id
            readers: Readers
            formattedTime: MessagesAdapter.getFormattedTime(Timestamp)
            extraHeight: progressBar.visible ? 18 : 0
            innerContent.children: [
                RowLayout {
                    id: transferItem
                    spacing: 6
                    anchors.right: isOutgoing ? parent.right : undefined
                    HoverHandler {
                        target: parent
                        enabled: canOpen
                        onHoveredChanged: {
                            dataTransferItem.hoveredLink = enabled && hovered ?
                                        ("file:///" + Body) : ""
                        }
                        cursorShape: enabled ?
                                         Qt.PointingHandCursor :
                                         Qt.ArrowCursor
                    }
                    Loader {
                        id: buttonsLoader

                        property string iconSourceA
                        property string iconSourceB

                        Layout.margins: 12

                        sourceComponent: {
                            switch (Status) {
                            case Interaction.Status.TRANSFER_CANCELED:
                            case Interaction.Status.TRANSFER_ERROR:
                            case Interaction.Status.TRANSFER_UNJOINABLE_PEER:
                            case Interaction.Status.TRANSFER_TIMEOUT_EXPIRED:
                                iconSourceA = JamiResources.error_outline_black_24dp_svg
                                return terminatedComp
                            case Interaction.Status.TRANSFER_CREATED:
                            case Interaction.Status.TRANSFER_FINISHED:
                                iconSourceA = JamiResources.link_black_24dp_svg
                                return terminatedComp
                            case Interaction.Status.TRANSFER_AWAITING_HOST:
                                iconSourceA = JamiResources.download_black_24dp_svg
                                iconSourceB = JamiResources.close_black_24dp_svg
                                return optionsComp
                            case Interaction.Status.TRANSFER_ONGOING:
                                iconSourceA = JamiResources.close_black_24dp_svg
                                return optionsComp
                            default:
                                iconSourceA = JamiResources.error_outline_black_24dp_svg
                                return terminatedComp
                            }
                        }
                        Component {
                            id: terminatedComp
                            ResponsiveImage {
                                source: buttonsLoader.iconSourceA
                                Layout.leftMargin: 12
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                            }
                        }
                        Component {
                            id: optionsComp
                            ColumnLayout {
                                Layout.leftMargin: 12
                                PushButton {
                                    source: buttonsLoader.iconSourceA
                                    normalColor: JamiTheme.chatviewBgColor
                                    imageColor: JamiTheme.chatviewButtonColor
                                    onClicked: {
                                        switch (Status) {
                                        case Interaction.Status.TRANSFER_ONGOING:
                                            return MessagesAdapter.cancelFile(Id)
                                        case Interaction.Status.TRANSFER_AWAITING_HOST:
                                            return MessagesAdapter.acceptFile(Id)
                                        default: break
                                        }
                                    }
                                }
                                PushButton {
                                    visible: !CurrentConversation.isSwarm
                                    height: visible * implicitHeight
                                    source: buttonsLoader.iconSourceB
                                    normalColor: JamiTheme.chatviewBgColor
                                    imageColor: JamiTheme.chatviewButtonColor
                                    onClicked: {
                                        switch (Status) {
                                        case Interaction.Status.TRANSFER_AWAITING_HOST:
                                            return MessagesAdapter.cancelFile(Id)
                                        default: break
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Column {
                        Layout.rightMargin: 24
                        spacing: 6
                        TextEdit {
                            id: transferName
                            width: Math.min(implicitWidth, maxMsgWidth)
                            topPadding: 10
                            text: CurrentConversation.isSwarm ?
                                      TransferName :
                                      Body
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                            font.weight: Font.DemiBold
                            font.pointSize: 11
                            renderType: Text.NativeRendering
                            readOnly: true
                            color: isOutgoing ?
                                       JamiTheme.messageOutTxtColor :
                                       JamiTheme.messageInTxtColor
                            MouseArea {
                                anchors.fill: parent
                                propagateComposedEvents: true
                                cursorShape: canOpen ?
                                                 Qt.PointingHandCursor :
                                                 Qt.ArrowCursor
                            }
                        }
                        Label {
                            id: transferInfo
                            width: Math.min(implicitWidth, maxMsgWidth)
                            bottomPadding: 10
                            text: {
                                var res = formattedTime + " - "
                                if (transferStats.totalSize !== undefined) {
                                    if (transferStats.progress !== 0 &&
                                            transferStats.progress !== transferStats.totalSize) {
                                        res += UtilsAdapter.humanFileSize(transferStats.progress) + " / "
                                    }
                                    res += UtilsAdapter.humanFileSize(transferStats.totalSize)
                                }
                                return res + " - " + MessagesAdapter.getStatusString(Status)
                            }
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                            font.pointSize: 10
                            renderType: Text.NativeRendering
                            color: Qt.lighter((isOutgoing ?
                                       JamiTheme.messageOutTxtColor :
                                       JamiTheme.messageInTxtColor), 1.5)
                        }
                    }
                }
                ,ProgressBar {
                    id: progressBar
                    visible: Status === Interaction.Status.TRANSFER_ONGOING
                    height: visible * implicitHeight
                    value: transferStats.progress / transferStats.totalSize
                    width: transferItem.width
                    anchors.right: isOutgoing ? parent.right : undefined
                }
            ]
        }
    }

    Component {
        id: localMediaMsgComp

        SBSMessageBase {
            id: localMediaMsgItem

            isOutgoing: Author === ""
            showTime: root.showTime
            seq: root.seq
            author: Author
            location: Body
            transferName: TransferName
            transferId: Id
            readers: Readers
            formattedTime: MessagesAdapter.getFormattedTime(Timestamp)
            bubble.visible: false
            innerContent.children: [
                Loader {
                    id: localMediaCompLoader
                    anchors.right: isOutgoing ? parent.right : undefined
                    width: sourceComponent.width
                    height: sourceComponent.height
                    sourceComponent: mediaInfo.isImage !== undefined ?
                                         imageComp :
                                         avComp
                    Component {
                        id: avComp
                        WebEngineView {
                            id: wev
                            anchors.right: isOutgoing ? parent.right : undefined
                            readonly property real minSize: 192
                            readonly property real maxSize: 256
                            readonly property real aspectRatio: 1 / .75
                            readonly property real adjustedWidth: Math.min(maxSize,
                                                                  Math.max(minSize,
                                                                           innerContent.width - senderMargin))
                            width: isFullScreen ? parent.width : adjustedWidth
                            height: mediaInfo.isVideo ?
                                        isFullScreen ?
                                            parent.height :
                                            Math.ceil(adjustedWidth / aspectRatio) :
                                        54
                            settings.fullScreenSupportEnabled: mediaInfo.isVideo
                            settings.javascriptCanOpenWindows: false
                            Component.onCompleted: loadHtml(mediaInfo.html, 'file://')
                            layer.enabled: parent !== appContainer && !appWindow.isFullScreen
                            layer.effect: OpacityMask {
                                maskSource: MessageBubble {
                                    out: isOutgoing
                                    type: seq
                                    width: wev.width
                                    height: wev.height
                                    radius: msgRadius
                                }
                            }
                            onFullScreenRequested: function(request) {
                                if (JamiQmlUtils.callIsFullscreen)
                                    return
                                if (request.toggleOn && !appWindow.isFullScreen) {
                                    parent = appContainer
                                    appWindow.toggleFullScreen()
                                } else if (!request.toggleOn && appWindow.isFullScreen) {
                                    parent = localMediaCompLoader
                                    appWindow.toggleFullScreen()
                                }
                                request.accept()
                            }
                        }
                    }
                    Component {
                        id: imageComp
                        AnimatedImage {
                            id: img
                            anchors.right: isOutgoing ? parent.right : undefined
                            property real minSize: 192
                            property real maxSize: 256
                            cache: false
                            fillMode: Image.PreserveAspectCrop
                            mipmap: true
                            antialiasing: true
                            autoTransform: false
                            asynchronous: true
                            source: "file:///" + Body
                            property real aspectRatio: implicitWidth / implicitHeight
                            property real adjustedWidth: Math.min(maxSize,
                                                                  Math.max(minSize,
                                                                           innerContent.width - senderMargin))
                            width: adjustedWidth
                            height: Math.ceil(adjustedWidth / aspectRatio)
                            Rectangle {
                                color: JamiTheme.previewImageBackgroundColor
                                z: -1
                                anchors.fill: parent
                            }
                            layer.enabled: true
                            layer.effect: OpacityMask {
                                maskSource: MessageBubble {
                                    out: isOutgoing
                                    type: seq
                                    width: img.width
                                    height: img.height
                                    radius: msgRadius
                                }
                            }
                            HoverHandler {
                                target : parent
                                onHoveredChanged: {
                                    localMediaMsgItem.hoveredLink = hovered ? img.source : ""
                                }
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }
            ]
        }
    }
}
