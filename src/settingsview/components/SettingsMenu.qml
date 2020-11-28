/*
 * Copyright (C) 2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import net.jami.Models 1.0

// TODO: these includes should generally be resource uris
import "../../commoncomponents"
import "../../settingsview"

Rectangle {
    id: root

    signal itemSelected(int index)

    Component.onCompleted: {
        listModel.append({ 'type': SettingsView.Account, 'name': qsTr("Account"),
                         'iconSource': "qrc:/images/icons/baseline-people-24px.svg"})
        listModel.append({ 'type': SettingsView.General, 'name': qsTr("General"),
                         'iconSource': "qrc:/images/icons/round-settings-24px.svg"})
        listModel.append({ 'type': SettingsView.Media, 'name': qsTr("Audio/Video"),
                         'iconSource': "qrc:/images/icons/baseline-desktop_windows-24px.svg"})
        listModel.append({ 'type': SettingsView.Plugin, 'name': qsTr("Plugin"),
                         'iconSource': "qrc:/images/icons/extension_24dp.svg"})
    }

    anchors.fill: parent
    color: JamiTheme.backgroundColor

    ButtonGroup {
        buttons: buttons.children
        onCheckedButtonChanged: itemSelected(checkedButton.menuType)
    }

    Column {
        id: buttons

        spacing: 0
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        Repeater {
            id: repeater

            model: ListModel { id: listModel }

            PushButton {
                property int menuType: type

                Component.onCompleted: checked = type === SettingsView.Account

                width: root.width
                height: 64

                buttonText: name
                source: iconSource
                imageColor: JamiTheme.textColor
                pressedColor: Qt.lighter(JamiTheme.pressedButtonColor, 1.25)
                checkedColor: JamiTheme.selectedColor
                hoveredColor: JamiTheme.hoverColor
                fontPointSize: JamiTheme.textFontSize + 2
                duration: 0
                textHAlign: Text.AlignLeft
                preferredMargin: 24
                normalColor: root.color
                checkable: true
                radius: 0
            }
        }
    }
}

