/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

Popup {
    id: root

    y: parent.height
    implicitWidth: parent.width
    // limit the number of accounts shown at once
    implicitHeight: {
        return visible ? Math.min(
                             JamiTheme.accountListItemHeight * Math.min(
                                 5, CurrentAccountFilterModel.rowCount() + 1),
                             mainViewSidePanelRect.height) : 0
    }
    padding: 0
    modal: true
    Overlay.modal: Rectangle {
        color: JamiTheme.transparentColor
    }

    contentItem: ColumnLayout {
        spacing: 0

        JamiListView {
            id: listView

            Layout.fillHeight: true
            Layout.preferredWidth: parent.width

            model: CurrentAccountFilterModel
            delegate: AccountItemDelegate {
                height: JamiTheme.accountListItemHeight
                width: root.width
                onClicked: {
                    root.close()
                    LRCInstance.currentAccountId = ID
                }
            }
        }

        // fake footer item as workaround for Qt 5.15 bug
        // https://bugreports.qt.io/browse/QTBUG-85302
        // don't use the clip trick and footer item overlay
        // explained here https://stackoverflow.com/a/64625149
        // as it causes other complexities in handling the drop shadow
        ItemDelegate {
            id: footerItem

            Layout.preferredHeight: JamiTheme.accountListItemHeight
            Layout.preferredWidth: parent.width

            background: Rectangle {
                color: footerItem.hovered?
                           JamiTheme.hoverColor :
                           JamiTheme.backgroundColor

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Add Account") + "+"
                    color: JamiTheme.textColor
                    font.pointSize: JamiTheme.textFontSize
                }
            }

            onClicked: {
                root.close()
                mainView.startWizard()
            }
        }
    }

    background: Rectangle {
        color: JamiTheme.backgroundColor
        CustomBorder {
            commonBorder: false
            tBorderwidth: 1; lBorderwidth: 2
            bBorderwidth: 2; rBorderwidth: 1
            borderColor: JamiTheme.tabbarBorderColor
        }

        layer {
            enabled: true
            effect: DropShadow {
                horizontalOffset: 3.0
                verticalOffset: 3.0
                radius: 16.0
                color: JamiTheme.shadowColor
                transparentBorder: true
            }
        }
    }
}
