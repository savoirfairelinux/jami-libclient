/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Trevor Tabah <trevor.tabah@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1
import net.jami.Enums 1.1

SBSMessageBase {
    id : root

    property bool isRemoteImage
    property real maxMsgWidth: root.width - senderMargin - 2 * hPadding - avatarBlockWidth

    isOutgoing: Author === ""
    author: Author
    readers: Readers
    formattedTime: MessagesAdapter.getFormattedTime(Timestamp)
    extraHeight: extraContent.active && !isRemoteImage ? msgRadius : -isRemoteImage
    innerContent.children: [
        TextEdit {
            padding: 10
            anchors.right: isOutgoing ? parent.right : undefined
            text: '<span style="white-space: pre-wrap">' + Body + '</span>'
            width: {
                if (extraContent.active)
                    Math.max(extraContent.width,
                             Math.min(implicitWidth - avatarBlockWidth,
                                      extraContent.minSize) - senderMargin)
                else
                    Math.min(implicitWidth, innerContent.width - senderMargin)
            }
            height: implicitHeight
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            selectByMouse: true
            font.pointSize: 11
            font.hintingPreference: Font.PreferNoHinting
            renderType: Text.NativeRendering
            textFormat: TextEdit.RichText
            onLinkHovered: root.hoveredLink = hoveredLink
            onLinkActivated: Qt.openUrlExternally(hoveredLink)
            readOnly: true
            color: isOutgoing ?
                       JamiTheme.messageOutTxtColor :
                       JamiTheme.messageInTxtColor
        },
        Loader {
            id: extraContent
            width: sourceComponent.width
            height: sourceComponent.height
            anchors.right: isOutgoing ? parent.right : undefined
            property real minSize: 192
            property real maxSize: 320
            active: LinkPreviewInfo.url !== undefined
            sourceComponent: ColumnLayout {
                id: previewContent
                spacing: 12
                Component.onCompleted: {
                    isRemoteImage = MessagesAdapter.isRemoteImage(LinkPreviewInfo.url)
                }
                HoverHandler {
                    target: previewContent
                    onHoveredChanged: {
                        root.hoveredLink = hovered ? LinkPreviewInfo.url : ""
                    }
                    cursorShape: Qt.PointingHandCursor
                }
                AnimatedImage {
                    id: img
                    cache: false
                    source: isRemoteImage ?
                                LinkPreviewInfo.url :
                                (hasImage ? LinkPreviewInfo.image : "")

                    fillMode: Image.PreserveAspectCrop
                    mipmap: true
                    antialiasing: true
                    autoTransform: true
                    asynchronous: true
                    readonly property bool hasImage: LinkPreviewInfo.image !== null
                    property real aspectRatio: implicitWidth / implicitHeight
                    property real adjustedWidth: Math.min(extraContent.maxSize,
                                                          Math.max(extraContent.minSize,
                                                                   maxMsgWidth))
                    Layout.preferredWidth: adjustedWidth
                    Layout.preferredHeight: Math.ceil(adjustedWidth / aspectRatio)
                    Rectangle {
                        color: JamiTheme.previewImageBackgroundColor
                        z: -1
                        anchors.fill: parent
                    }
                    layer.enabled: isRemoteImage
                    layer.effect: OpacityMask {
                        maskSource: MessageBubble {
                            Rectangle { height: msgRadius; width: parent.width }
                            out: isOutgoing
                            type: seq
                            width: img.width
                            height: img.height
                            radius: msgRadius
                        }
                    }
                }
                Column {
                    opacity: img.status !== Image.Loading
                    visible: !isRemoteImage
                    Layout.preferredWidth: img.width - 2 * hPadding
                    Layout.leftMargin: hPadding
                    Layout.rightMargin: hPadding
                    spacing: 6
                    Label {
                        width: parent.width
                        font.pointSize: 10
                        font.hintingPreference: Font.PreferNoHinting
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        renderType: Text.NativeRendering
                        textFormat: TextEdit.RichText
                        color: JamiTheme.previewTitleColor
                        visible: LinkPreviewInfo.title !== null
                        text: LinkPreviewInfo.title
                    }
                    Label {
                        width: parent.width
                        font.pointSize: 11
                        font.hintingPreference: Font.PreferNoHinting
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        renderType: Text.NativeRendering
                        textFormat: TextEdit.RichText
                        color: JamiTheme.previewSubtitleColor
                        visible: LinkPreviewInfo.description !== null
                        text: '<a href=" " style="text-decoration: ' +
                              ( hoveredLink ? 'underline' : 'none') + ';"' +
                              '>' + LinkPreviewInfo.description + '</a>'
                    }
                    Label {
                        width: parent.width
                        font.pointSize: 10
                        font.hintingPreference: Font.PreferNoHinting
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        renderType: Text.NativeRendering
                        textFormat: TextEdit.RichText
                        color: JamiTheme.previewSubtitleColor
                        text: LinkPreviewInfo.domain
                    }
                }
            }
        }
    ]

    opacity: 0
    Behavior on opacity { NumberAnimation { duration: 100 } }
    Component.onCompleted: {
        if (!Linkified) {
            MessagesAdapter.parseMessageUrls(Id, Body, UtilsAdapter.getAppValue(Settings.DisplayHyperlinkPreviews))
        }
        opacity = 1
    }
}
